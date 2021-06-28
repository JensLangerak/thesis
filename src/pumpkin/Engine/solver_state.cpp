#include "solver_state.h"
#include "../../logger/logger.h"
#include "../Utilities/runtime_assert.h"
#include "variable_selector.h"

#include <algorithm>
#include <assert.h>
#include <iostream>

namespace Pumpkin
{

SolverState::SolverState(int64_t num_Boolean_variables, ParameterHandler&params):
	variable_selector_(num_Boolean_variables, params.GetFloatParameter("decay-factor-variables")),
	value_selector_(num_Boolean_variables),
	assignments_(num_Boolean_variables), //note that the 0th position is not used
	//propagator_clausal_binary_(num_Boolean_variables * params.GetBooleanParameter("binary-clause-propagator")),
	propagator_clausal_(num_Boolean_variables, params.GetFloatParameter("decay-factor-learned-clause"), params.GetIntegerParameter("lbd-threshold"), params.GetIntegerParameter("limit-num-temporary-clauses"), params.GetBooleanParameter("lbd-sorting-temporary-clauses")),
        propagator_pseudo_boolean_(num_Boolean_variables),
	decision_level_(0),
	simple_moving_average_lbd(params.GetIntegerParameter("glucose-queue-lbd-limit")),
	simple_moving_average_block(params.GetIntegerParameter("glucose-queue-reset-limit")),
	use_binary_clause_propagator_(params.GetBooleanParameter("binary-clause-propagator")),
	saved_state_num_permanent_clauses_(-1)
{
  //TODO create additional propagators
}

SolverState::~SolverState()
{
	for (PropagatorGeneric* propagator : additional_propagators_) { delete propagator; }
}

void SolverState::EnqueueDecisionLiteral(BooleanLiteral decision_literal)
{
	assert(assignments_.IsAssigned(decision_literal) == false);
	MakeAssignment(decision_literal, NULL, NULL);
}

bool SolverState::EnqueuePropagatedLiteral(BooleanLiteral literal, PropagatorGeneric *responsible_propagator, uint64_t code)
{
	assert(responsible_propagator != NULL);
	if (assignments_.IsAssigned(literal) == true) //we are trying to propagate a variable that has already been assigned a value
	{		
		return assignments_.GetAssignment(literal.Variable()) == literal; //return if the assignment is consistent with the propagation
	}
	else
	{
		MakeAssignment(literal, responsible_propagator, code);
		return true;
	}
}

PropagatorGeneric* SolverState::PropagateEnqueued()
{
	//clauses are propagated until completion
	//for the remaining propagators, in order of propagators: one literal is considered, and if it propagates, the process restarts from the clausal propagator, otherwise the next propagator is considered for propagation.
	//the intuition is to perform simpler propagations first
	while (IsPropagationComplete() == false)
	{
		bool conflict_detected = !propagator_clausal_.Propagate(*this);
		if (conflict_detected) { return &propagator_clausal_; }

		for (PropagatorGeneric* propagator : additional_propagators_)
		{
			int num_assignments_before = GetNumberOfAssignedVariables();
			conflict_detected = !propagator->PropagateOneLiteral(*this);
			if (conflict_detected) { return propagator; }
			if (num_assignments_before != GetNumberOfAssignedVariables()) { break; } //if at least one literal was propagated, go to the clausal propagator
		}
	}
	return NULL;
}

void SolverState::IncreaseDecisionLevel()
{
	decision_level_++;
	trail_delimiter_.push_back(int(trail_.size()));
}

void SolverState::Backtrack(int backtrack_level)
{
	runtime_assert(backtrack_level >= 0 && backtrack_level < decision_level_);
	
	while (decision_level_ != backtrack_level)
	{
		BacktrackOneLevel();
	}
	//propagator_clausal_binary_.Synchronise(*this);
	propagator_clausal_.Synchronise(*this);
	for (PropagatorGeneric* propagator : additional_propagators_) { propagator->Synchronise(*this); }	
}

void SolverState::Reset()
{
	if (GetCurrentDecisionLevel() != 0) Backtrack(0);
        AddScheduledEncodings();
}

void SolverState::BacktrackOneLevel()
{
	int num_assignments_for_removal = int(trail_.size() - trail_delimiter_.back());
	assert(num_assignments_for_removal >= 0);
	for (int i = 0; i < num_assignments_for_removal; i++)
	{
		UndoLastAssignment();
	}
	trail_delimiter_.pop_back();
	decision_level_--;
}

void SolverState::UndoLastAssignment()
{
	BooleanVariable last_assigned_variable = trail_.back().Variable();
	variable_selector_.Readd(last_assigned_variable);
	value_selector_.UpdatePolarity(last_assigned_variable, assignments_.IsAssignedTrue(last_assigned_variable));
	assignments_.UnassignVariable(last_assigned_variable);
	trail_.pop_back();
}

BooleanLiteral SolverState::getLastDecisionLiteralOnTrail() const
{
	return GetDecisionLiteralForLevel(GetCurrentDecisionLevel());
}

BooleanLiteral SolverState::GetDecisionLiteralForLevel(int decision_level) const
{
	runtime_assert(decision_level <= GetCurrentDecisionLevel());

	if (decision_level == 0) { return BooleanLiteral(); } //return undefined literal when there are no decisions on the trail

	BooleanLiteral decision_literal = trail_[trail_delimiter_[decision_level - 1]];
	
	assert(assignments_.GetAssignmentPropagator(decision_literal.Variable()) == NULL);
	assert(assignments_.GetAssignmentLevel(decision_literal.Variable()) == decision_level);
	return decision_literal;
}

int SolverState::GetCurrentDecisionLevel() const
{
	return decision_level_;
}

size_t SolverState::GetNumberOfAssignedVariables() const
{
	return int(trail_.size());
}

size_t SolverState::GetNumberOfVariables() const
{
	return int(assignments_.GetNumberOfVariables());
}

BooleanLiteral SolverState::GetLiteralFromTrailAtPosition(size_t index) const
{
	return trail_[index];
}

BooleanLiteral SolverState::GetLiteralFromTheBackOfTheTrail(size_t index) const
{
	return trail_[trail_.size() - index - 1];
}

BooleanAssignmentVector SolverState::GetOutputAssignment() const
{
	BooleanAssignmentVector output(GetNumberOfVariables());
	for (int i = 1; i <= GetNumberOfVariables(); i++)
	{
		runtime_assert(assignments_.IsAssigned(BooleanVariable(i)));
		output[i] = assignments_.GetAssignment(BooleanVariable(i)).IsPositive();
	}
	return output;
}

bool SolverState::IsAssignmentBuilt()
{
	return variable_selector_.PeekNextVariable(this) == BooleanVariable(); //no variables are left unassigned, the assignment is built
}

void SolverState::PrintTrail() const
{
	std::cout << "Trail\n";
	for (int i = 0; i < GetNumberOfAssignedVariables(); i++)
	{
		std::cout << GetLiteralFromTheBackOfTheTrail(i).VariableIndex() << "\n";
	}
	std::cout << "end trail\n";
}

BooleanVariable SolverState::CreateNewVariable()
{
	BooleanVariable new_variable(GetNumberOfVariables() + 1);

	variable_selector_.Grow();
	value_selector_.Grow();
	assignments_.Grow();
	//if (use_binary_clause_propagator_) { propagator_clausal_binary_.Grow(); }
	propagator_clausal_.clause_database_.watch_list_.Grow();
        for (auto p : additional_propagators_)
          p->GrowDatabase();

	return new_variable;
}

void SolverState::CreateVariablesUpToIndex(int largest_variable_index)
{
	for (int i = GetNumberOfVariables() + 1; i <= largest_variable_index; i++) 
	{ 
		CreateNewVariable(); 
	}
}

bool SolverState::AddUnitClause(BooleanLiteral literal)
{
	if (GetCurrentDecisionLevel() != 0) { Backtrack(0); }
	if (assignments_.IsAssigned(literal)) { return assignments_.IsAssignedTrue(literal); }

	EnqueueDecisionLiteral(literal);
	PropagatorGeneric* conflict_propagator = PropagateEnqueued();

	return conflict_propagator == NULL;
}

bool SolverState::AddUnitClauses(std::vector<BooleanLiteral>& units)
{
	for (BooleanLiteral literal : units) 
	{ 
		bool success = AddUnitClause(literal);
		if (!success) { return false; }
	}
	return true;
}

PropagatorGeneric* SolverState::AddClause(std::vector<BooleanLiteral>& literals)
{
	runtime_assert(IsPropagationComplete());
	runtime_assert(literals.size() > 1); //for now we do not allow adding a unit clause using this method because it's unclear what's the correct output if the unit clause was already set at the root to the opposite value	

	if (1==2 && literals.size() == 2 && use_binary_clause_propagator_)
	{
		return NULL;// propagator_clausal_binary_.AddClause(literals[0], literals[1], *this);
	}
	else
	{
		return propagator_clausal_.AddPermanentClause(literals, *this);
	}	
}

PropagatorGeneric* SolverState::AddBinaryClause(BooleanLiteral a, BooleanLiteral b)
{
	helper_vector_.clear();
	helper_vector_.push_back(a);
	helper_vector_.push_back(b);
	return AddClause(helper_vector_);
}

PropagatorGeneric* SolverState::AddTernaryClause(BooleanLiteral a, BooleanLiteral b, BooleanLiteral c)
{
	helper_vector_.clear();
	helper_vector_.push_back(a);
	helper_vector_.push_back(b);
	helper_vector_.push_back(c);
	return AddClause(helper_vector_);
}

PropagatorGeneric* SolverState::AddImplication(BooleanLiteral a, BooleanLiteral b)
{
	return AddBinaryClause(~a, b);
}

int SolverState::AddPropagator(PropagatorGeneric* propagator)
{
	additional_propagators_.push_back(propagator);
	return additional_propagators_.size() - 1;
}

PropagatorGeneric* SolverState::GetPropagator(int propagator_id)
{
	return additional_propagators_[propagator_id];
}

bool SolverState::HasPropagator(PropagatorGeneric* propagator)
{
	bool found = &propagator_clausal_ == propagator;
	for (PropagatorGeneric* stored_propagator : additional_propagators_) { found |= (stored_propagator == propagator); }
	return found;
}

TwoWatchedClause* SolverState::AddLearnedClauseToDatabase(std::vector<BooleanLiteral>& literals)
{
	TwoWatchedClause* learned_clause = this->propagator_clausal_.clause_database_.AddLearnedClause(literals, *this);
	this->propagator_clausal_.clause_database_.BumpClauseActivity(learned_clause);
	return learned_clause; 
}

void SolverState::UpdateMovingAveragesForRestarts(int learned_clause_lbd)
{
	simple_moving_average_lbd.AddTerm(learned_clause_lbd);
	cumulative_moving_average_lbd.AddTerm(learned_clause_lbd);
	simple_moving_average_block.AddTerm(GetNumberOfAssignedVariables());
}

void SolverState::SetStateResetPoint()
{
	runtime_assert(decision_level_ == 0);
	runtime_assert(use_binary_clause_propagator_ == false); //for now we did not take into account the clauses added to the binary clause propagator, so we must have it false
	saved_state_num_permanent_clauses_ = propagator_clausal_.clause_database_.permanent_clauses_.size();
	saved_state_num_root_literal_assignments_ = trail_.size();
}

void SolverState::PerformStateReset()
{
	runtime_assert(decision_level_ == 0);
	runtime_assert(saved_state_num_permanent_clauses_ != -1);
	runtime_assert(propagator_clausal_.clause_database_.permanent_clauses_.size() >= saved_state_num_permanent_clauses_);
	
	int64_t num_removed_clauses = 0;
	int64_t num_removed_unit_clauses = 0;

	while (propagator_clausal_.clause_database_.permanent_clauses_.size() != saved_state_num_permanent_clauses_)
	{
		TwoWatchedClause* clause = propagator_clausal_.clause_database_.permanent_clauses_.back();
		propagator_clausal_.clause_database_.RemoveWatchers(clause);
		propagator_clausal_.clause_database_.permanent_clauses_.pop_back();
		delete clause;
		num_removed_clauses++;
	}

	runtime_assert(trail_delimiter_.size() == 0);
	runtime_assert(trail_.size() >= saved_state_num_root_literal_assignments_);
	while (trail_.size() != saved_state_num_root_literal_assignments_)
	{
		num_removed_unit_clauses++;
		UndoLastAssignment();
	}

	int64_t num_removed_learned_clauses = 0;
	//only remove learned clauses if we actually did something since the last check point
	std::cout << "c TODO: state reset might be problematic in general, fix\n";
	//	the problem is that it could be that solve added some learned clauses since last time even though the IF statement is satistified
	//	for the current version of the code this works fine, but in general it might be problematic
	//	a correct version would be to check if the learned clauses are identical to when we saved the state last time
	//		however this would mean that we need to store a copy of the clauses
	//		probably need a better way to do this
	if (num_removed_clauses > 0 || num_removed_unit_clauses > 0)
	{
		num_removed_learned_clauses = propagator_clausal_.clause_database_.NumLearnedClauses();
		propagator_clausal_.clause_database_.RemoveAllLearnedClauses();
	}

	propagator_clausal_.Synchronise(*this);
	for (PropagatorGeneric* propagator : additional_propagators_) { propagator->Synchronise(*this); }
	variable_selector_.Reset(); //could use different seeds here..perhaps every time a reset happens we increment the seed by one?

	std::cout << "c state reset, removed " << num_removed_clauses << " hard clauses, " << num_removed_unit_clauses << " unit clauses, and " << num_removed_learned_clauses << " learned clauses\n";
}

void SolverState::MakeAssignment(BooleanLiteral literal, PropagatorGeneric *responsible_propagator, uint64_t code)
{
	assert(literal.IsUndefined() == false);
	assert(assignments_.IsAssigned(literal) == false);

	assignments_.MakeAssignment(literal.Variable(), literal.IsPositive(), GetCurrentDecisionLevel(), responsible_propagator, code, trail_.size());
	trail_.push_back(literal);
}

bool SolverState::IsPropagationComplete()
{
	bool propagation_complete = propagator_clausal_.IsPropagationComplete(*this);
	for (PropagatorGeneric* propagator : additional_propagators_) { propagation_complete &= propagator->IsPropagationComplete(*this); }
	return propagation_complete;
}
void SolverState::AddScheduledEncodings() {
  int start_lits = GetNumberOfVariables();
  for (auto c : scheduled_dynamic_constraints_) {
    c->AddScheduledEncoding(*this);
  }
  scheduled_dynamic_constraints_.clear();
  if (GetNumberOfVariables() > start_lits)
    simple_sat_solver::logger::Logger::Log2("nr lits: " + std::to_string(GetNumberOfVariables()));
}


} //end Pumpkin namespace