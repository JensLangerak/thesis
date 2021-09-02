#include "constraint_satisfaction_solver.h"
#include "../../logger/logger.h"
#include "../Propagators/reason_generic.h"
#include "../Utilities/runtime_assert.h"

#include <algorithm>
#include <assert.h>
#include <iostream>
#include "../Propagators/Dynamic/PseudoBoolean/propagator_pseudo_boolean_2.h"
#include "../Propagators/Dynamic/Encoders/generalized_totaliser.h"
#include "../Propagators/Dynamic/ExtendedGroups/propagator_extended_groups.h"

namespace Pumpkin
{
ConstraintSatisfactionSolver::ConstraintSatisfactionSolver(ParameterHandler& parameters):
	state_(0, parameters),
	internal_parameters_(parameters),
	counters_(parameters),
	seen_(1),//the 0-th entry is not used
	backtrack_level_(0),
	num_current_decision_level_literals_(0),
	num_trail_literals_examined_(0),
	learned_clause_minimiser_(state_), //todo need to ensure that the state object is created before the clause minimiser, rethink the initialisation strategy
	already_processed_(0),
	use_glucose_bumping_(false),
	use_clause_minimisation_(parameters.GetBooleanParameter("clause-minimisation"))
{
}

ConstraintSatisfactionSolver::ConstraintSatisfactionSolver(ProblemSpecification* problem_specification, ParameterHandler& parameters):
	state_(problem_specification->num_Boolean_variables_, parameters),
	internal_parameters_(parameters),
	counters_(parameters),
	seen_(problem_specification->num_Boolean_variables_ + 1),//the 0-th entry is not used
	backtrack_level_(0),
	num_current_decision_level_literals_(0),
	num_trail_literals_examined_(0),
	learned_clause_minimiser_(state_), //todo need to ensure that the state object is created before the clause minimiser, rethink the initialisation strategy
	already_processed_(problem_specification->num_Boolean_variables_),
	use_glucose_bumping_(false),
	use_clause_minimisation_(parameters.GetBooleanParameter("clause-minimisation"))
{
  for (BooleanLiteral unit_literal : problem_specification->unit_clauses_) { state_.AddUnitClause(unit_literal); }
	for (auto& clause : problem_specification->clauses_) { state_.AddClause(clause); }
        for (auto pb : problem_specification->pseudo_boolean_constraints_) {
          pb.pseudo_boolean_adder
              ->AddConstraintWithModifier(pb, state_);
        }

         state_.variable_selector_.Reset(parameters.GetIntegerParameter("seed"));
}

  SolverOutput ConstraintSatisfactionSolver::Solve(double time_limit_in_seconds)
{
	std::vector<BooleanLiteral> empty_assumptions;
	return Solve(empty_assumptions, time_limit_in_seconds);
}

SolverOutput ConstraintSatisfactionSolver::Solve(std::vector<BooleanLiteral>& assumptions, double time_limit_in_seconds)
{
	SolverExecutionFlag flag = SolveInternal(assumptions, time_limit_in_seconds);
        LogUsedVars();

//	std::cout << "c conflicts until restart: " << counters_.conflicts_until_restart << "\n";
//	std::cout << "c restart counter: " << counters_.num_restarts << "\n";

	double runtime = stopwatch_.TimeElapsedInSeconds();
	bool timeout = (flag == SolverExecutionFlag::TIMEOUT);
	int64_t cost = -1;
	BooleanAssignmentVector solution;
	Disjunction core;

	if (flag == SolverExecutionFlag::SAT)
	{
		cost = 0;
		solution = state_.GetOutputAssignment();

		//debug check to see if assumptions are satisfied
		for (BooleanLiteral assumption_literal : assumptions)
		{
			runtime_assert(solution[assumption_literal]);
		}
	}
	else if (flag == SolverExecutionFlag::UNSAT_UNDER_ASSUMPTIONS)
	{
		BooleanLiteral falsified_assumption = PeekNextDecisionLiteral();
		runtime_assert(state_.assignments_.IsAssignedFalse(falsified_assumption));
		core = ExtractCore(falsified_assumption);
	}

	CleanUp();
        std::cout << "restarts: " <<restarts_counter_debug << std::endl;
	return SolverOutput(runtime, timeout, solution, cost, core);
}

std::string ConstraintSatisfactionSolver::GetStatisticsAsString()
{
	std::string s;

	s += "c restarts: " + std::to_string(counters_.num_restarts);
	if (counters_.num_restarts == 0)
	{
		s += "\n";
	}
	else
	{
		s += " (" + std::to_string(counters_.conflicts / counters_.num_restarts) + " conflicts in avg)" + "\n";
	}
	s += "c blocked restarts: " + std::to_string(counters_.blocked_restarts) + "\n";
	s += "c nb removed clauses: " + std::to_string(state_.propagator_clausal_.clause_database_.counter_total_removed_clauses_) + "\n";
	s += "c nb learnts DL2: " + std::to_string(counters_.small_lbd_clauses_learned) + "\n";
	s += "c nb learnts size 1: " + std::to_string(counters_.unit_clauses_learned) + "\n";
	s += "c nb learnts size 3: " + std::to_string(counters_.ternary_clauses_learned) + "\n";
	s += "c nb learnts: " + std::to_string(state_.propagator_clausal_.clause_database_.number_of_learned_clauses_) + "\n";
	s += "c avg learnt clause size: " + std::to_string(double(state_.propagator_clausal_.clause_database_.number_of_learned_literals_) / state_.propagator_clausal_.clause_database_.number_of_learned_clauses_) + "\n";
	s += "c current number of learned clauses: " + std::to_string(state_.propagator_clausal_.clause_database_.NumLearnedClauses()) + "\n";
	s += "c ratio of learned clauses: " + std::to_string(double(state_.propagator_clausal_.clause_database_.NumLearnedClauses()) / state_.propagator_clausal_.clause_database_.NumClausesTotal()) + "\n";
	s += "c conflicts: " + std::to_string(counters_.conflicts) + "\n";
	s += "c decisions: " + std::to_string(counters_.decisions) + "\n";
	s += "c propagations: " + std::to_string(counters_.propagations) + "\n";

	return s;
}

ConstraintSatisfactionSolver::SolverExecutionFlag ConstraintSatisfactionSolver::SolveInternal(std::vector<BooleanLiteral>& assumptions, double time_limit_in_seconds)
{
	Initialise(time_limit_in_seconds, assumptions);
        simple_sat_solver::logger::Logger::Log2("nr lits: " + std::to_string(state_.GetNumberOfVariables()));
        int nr_clauses = state_.propagator_clausal_.clause_database_.permanent_clauses_.size();
        simple_sat_solver::logger::Logger::Log2("nr clauses: " + std::to_string(nr_clauses));


	//check failure by unit propagation at root level - perhaps this could be done as part of a preprocessing step?
  state_.propagator_clausal_.next_position_on_trail_to_propagate_ = 0;
  if (SetUnitClauses() == false) { return SolverExecutionFlag::UNSAT; }

	while (stopwatch_.IsWithinTimeLimit())
	{
		state_.IncreaseDecisionLevel();
		BooleanLiteral next_decision_literal = PeekNextDecisionLiteral();

		//a full assignment has been built, stop the search
		if (next_decision_literal.IsUndefined()) {
                  return SolverExecutionFlag::SAT; }
		//a decision literal may be already assigned only if it is a conflicting assumption, stop the search
		if (state_.assignments_.IsAssigned(next_decision_literal)) {
                  return SolverExecutionFlag::UNSAT_UNDER_ASSUMPTIONS; }

		int64_t num_assigned_variables_old = state_.GetNumberOfAssignedVariables();
		state_.EnqueueDecisionLiteral(next_decision_literal);
		PropagatorGeneric* conflicting_propagator = state_.PropagateEnqueued();
		counters_.propagations += (state_.GetNumberOfAssignedVariables() - num_assigned_variables_old);

		//check if a conflict has been encountered
		if (conflicting_propagator != NULL)
		{
			bool success = ResolveConflict(conflicting_propagator);
			//if the conflict could not be resolved, then UNSAT has been detected, stop the search
			if (success == false) {
                          return SolverExecutionFlag::UNSAT; }

			if (ShouldRestart()) { PerformRestartDuringSearch(); }
		} else {
//                  for (auto c : state_.propagator_clausal_.clause_database_.permanent_clauses_) {
//                    bool sat = false;
//                    for (auto b : c->literals_) {
//                      if (!state_.assignments_.IsAssignedFalse(b)) {
//                        sat = true;
//                        break;
//                      }
//                    }
//                    if (!sat) {
//                      auto test= c->GetWatchedLiterals();
//                      std::vector<BooleanLiteral> l;
//                      std::vector<int> assign_level;
//                      for (int i = 0; i < c->literals_.Size(); ++i) {
//                        l.push_back(c->literals_[i]);
//                        assign_level.push_back(state_.assignments_.GetAssignmentLevel(c->literals_[i]));
//                      }
//                      PerformRestartDuringSearch();

//                    }
//
//
//                  }

                }
	}
	return SolverExecutionFlag::TIMEOUT;
}

void ConstraintSatisfactionSolver::Initialise(double time_limit_in_seconds, std::vector<BooleanLiteral>& assumptions)
{
	stopwatch_.Initialise(time_limit_in_seconds);
	seen_.Resize(state_.GetNumberOfVariables() + 1); //todo ideally this should be adjusted when new variables are added, but for now this works
	assumptions_ = assumptions;
}

bool ConstraintSatisfactionSolver::AreAllAssumptionsSet()
{
	return state_.GetCurrentDecisionLevel() > assumptions_.size();
}

BooleanLiteral ConstraintSatisfactionSolver::PeekNextAssumption()
{
	assert(state_.GetCurrentDecisionLevel() > 0); //no assumptions can be set at decision level zero; level zero is reserved only for unit clauses

	if (AreAllAssumptionsSet()) { return BooleanLiteral::UndefinedLiteral(); }

	BooleanLiteral next_assumption = assumptions_[state_.GetCurrentDecisionLevel()-1];
	while (AreAllAssumptionsSet() == false && state_.assignments_.IsAssignedTrue(next_assumption))
	{
		state_.IncreaseDecisionLevel();
		if (AreAllAssumptionsSet()) { break; }
		next_assumption = assumptions_[state_.GetCurrentDecisionLevel() - 1];
	}

	//if all assumptions are assigned true, report that there are no assumptions left
	if (AreAllAssumptionsSet()) { return BooleanLiteral::UndefinedLiteral(); }

	assert(state_.assignments_.IsAssigned(next_assumption) == false || state_.assignments_.IsAssignedFalse(next_assumption));
	return next_assumption;
}

BooleanLiteral ConstraintSatisfactionSolver::PeekNextDecisionLiteral()
{
	//assumption literals have priority over other literals
	//	check if there are any assumption literals that need to be set
	BooleanLiteral next_assumption = PeekNextAssumption();
	if (!next_assumption.IsUndefined()) { return next_assumption; }
	//at this point, no assumptions need to be set
	//	proceed with standard variable selection
	BooleanVariable selected_variable = state_.variable_selector_.PeekNextVariable(&state_);
	//variable selector returns an undefined variable if every variable has been assigned
	if (selected_variable.IsUndefined()) { return BooleanLiteral::UndefinedLiteral(); }

	counters_.decisions++;

	bool selected_value = state_.value_selector_.SelectValue(selected_variable);
	BooleanLiteral decision_literal(selected_variable, selected_value);

	assert(state_.assignments_.IsAssigned(decision_literal) == false);
	return decision_literal;
}

Disjunction ConstraintSatisfactionSolver::ExtractCore(BooleanLiteral falsified_assumption)
{
	//todo the code needs to be cleaned up at some point; the code is adapted from a previous version

	//	find the explanation for the propagation of a false assumption
	//	turn the explanation into a clause
	//	use a procedure to replace propagated literals with decision literals
	//	turn the clause into a conjuction
	//	add the falsified assumption to the conjunction
	//	return the core

	already_processed_.Resize(state_.GetNumberOfVariables() + 1);
	already_processed_.Clear();

	if (state_.assignments_.IsRootAssignment(falsified_assumption)){ return Disjunction(1, ~falsified_assumption); }

	PropagatorGeneric* propagator = state_.assignments_.GetAssignmentPropagator(~falsified_assumption);

	//happens only if the assumptions are inconsistent, e.g., they contain both 'x' and '~x'
	if (propagator == NULL)
	{
		Disjunction core;
		core.push_back(~falsified_assumption);
		core.push_back(falsified_assumption);
		return core;
	}

	ExplanationGeneric* explanation = propagator->ExplainLiteralPropagation(~falsified_assumption, state_);

	//at this point, we know the learned clause contains only assumptions, but some assumptions might be implied
	//we now expand implied assumptions until all literals are decision assumptions
	Conjunction core_clause;
	std::vector<BooleanLiteral> unconsidered_literals;
	for (int i = 0; i < explanation->Size(); i++)
	{
		unconsidered_literals.push_back(~explanation->operator[](i));
		already_processed_.Insert(explanation->operator[](i).VariableIndex());
	}

	while (!unconsidered_literals.empty())
	{
		BooleanLiteral current_literal = unconsidered_literals.back();
		unconsidered_literals.pop_back();

		if (state_.assignments_.IsRootAssignment(current_literal))
		{
			//do nothing
		}
		else if (state_.assignments_.IsDecision(current_literal))
		{
			runtime_assert(already_processed_.IsPresent(current_literal.VariableIndex()));
			//unit clauses are added to the solver as decision at level zero
			//those need to be ignored
			//todo -> IsDecision should take that into account actually...not us here
			if (state_.assignments_.GetAssignmentLevel(current_literal) > 0)
			{
				core_clause.push_back(current_literal);
			}
		}
		else
		{//we now expand the reason for the implied assumption with other assumptions
			PropagatorGeneric* propagator = state_.assignments_.GetAssignmentPropagator(~current_literal);
			ExplanationGeneric* explanation = propagator->ExplainLiteralPropagation(~current_literal, state_);
			for (int i = 0; i < explanation->Size(); i++)
			{
				BooleanLiteral reason_literal = (*explanation)[i];
				runtime_assert(state_.assignments_.IsAssignedTrue(reason_literal));
				//ignore literals already processed: this may happen if an assumption was responsible for two or more other implied assumptions
				if (already_processed_.IsPresent(reason_literal.VariableIndex()) || state_.assignments_.GetAssignmentLevel(reason_literal) == 0) { continue; }

				unconsidered_literals.push_back(~reason_literal);
				already_processed_.Insert(reason_literal.VariableIndex());
			}
		}
	}
	core_clause.push_back(~falsified_assumption);

	//at this point, all literals in the core are non-implied assumptions
	already_processed_.Clear();

	//debug check that the core only contains assumptions
	for (BooleanLiteral core_literal : core_clause)
	{
		runtime_assert(state_.assignments_.GetAssignmentLevel(core_literal) <= assumptions_.size());
		runtime_assert(state_.assignments_.IsDecision(core_literal) || ~core_literal == falsified_assumption);
	}

	return core_clause;
}

bool ConstraintSatisfactionSolver::ResolveConflict(PropagatorGeneric *conflict_propagator)
{
	runtime_assert(conflict_propagator != NULL);

	while (conflict_propagator != NULL)
	{
                state_.propagator_clausal_.log_learned = false;
		UpdateConflictCounters();

		if (state_.GetCurrentDecisionLevel() == 0) { return false; } //conflict found at the root level - unsatisfiability is detected

		ConflictAnalysisResultClausal analysis_result = AnalyseConflict(conflict_propagator);

                if (state_.propagator_clausal_.log_learned)
                LogLearnedClause(analysis_result);

		conflict_propagator = ProcessConflictAnalysisResult(analysis_result);
	}
	state_.propagator_clausal_.clause_database_.DecayClauseActivities();
	state_.variable_selector_.DecayActivities();

	return true; //conflict was successful resolved
}

ConflictAnalysisResultClausal ConstraintSatisfactionSolver::AnalyseConflict(PropagatorGeneric* conflict_propagator)
{
	runtime_assert(conflict_propagator != NULL);
	assert(CheckConflictAnalysisDataStructures());

        //TODO connect to add new variable in state?
        if (seen_.GetCapacity() < state_.GetNumberOfVariables() +1)
          seen_.Resize(state_.GetNumberOfVariables() + 1);
        assert(seen_.GetCapacity() == state_.GetNumberOfVariables() + 1);

	use_glucose_bumping_ = false; //disabling glucose bumping functionality, will enable in the future

	//initialise the analysis with the conflict explanation
	//recall this updates learned_clause_literals_, the num_current_decision_level_literals_, and the backtrack_level_
	ProcessConflictPropagator(conflict_propagator, BooleanLiteral());

	//start inspecting the implication graph of conflict analysis
	while (num_current_decision_level_literals_ > 1)
	{
		BooleanLiteral resolution_literal = FindNextReasonLiteralOnTheTrail(); //recall this method will lower the num_current_decision_level_literals counter
		PropagatorGeneric* explanation_propagator = state_.assignments_.GetAssignmentPropagator(resolution_literal.Variable());
		ProcessConflictPropagator(explanation_propagator, resolution_literal);
	}

	//at this point, we know that we expanded all variables of the current decision level that are relevant to the conflict except one literal, which is the first unique implication point
	BooleanLiteral unique_implication_point_literal = FindNextReasonLiteralOnTheTrail();
	state_.variable_selector_.BumpActivity(unique_implication_point_literal.Variable());
	learned_clause_literals_.push_back(~unique_implication_point_literal);

//        state_.update_vars_analyzer_.insert(unique_implication_point_literal.VariableIndex());

        ConflictAnalysisResultClausal analysis_result(learned_clause_literals_, ~unique_implication_point_literal, backtrack_level_);
	assert(analysis_result.CheckCorrectnessAfterConflictAnalysis(state_));

	if (use_clause_minimisation_) { learned_clause_minimiser_.RemoveImplicationGraphDominatedLiterals(analysis_result); }
	if (use_glucose_bumping_) { PerformGlucoseVariableBumping(); }
	ClearConflictAnalysisDataStructures();

	return analysis_result;
}

void ConstraintSatisfactionSolver::ProcessConflictPropagator(PropagatorGeneric *conflict_propagator, BooleanLiteral propagated_literal)
{
        assert(seen_.GetCapacity() == state_.GetNumberOfVariables() + 1);
	runtime_assert(conflict_propagator != NULL);
	//analyse the reason_constraint of the conflict.
	//Add new literals to the learned clause_ if their decision level if lower than the current one
	//otherwise increase the number of the number of literals in the current decision level
	ExplanationGeneric *explanation;

	//undefine literal signals that this call is for the initial conflict clause step
	if (propagated_literal.IsUndefined())
	{
		explanation = conflict_propagator->ExplainFailure(state_);
		if (conflict_propagator == &state_.propagator_clausal_) { state_.propagator_clausal_.BumpFailureClause(state_); }
	}
	else //standard propagation step
	{
		explanation = conflict_propagator->ExplainLiteralPropagation(propagated_literal, state_);
		if (conflict_propagator == &state_.propagator_clausal_) { state_.propagator_clausal_.BumpPropagatingClause(propagated_literal, state_); }
	}

  for (int i = 0; i < explanation->Size(); i++)
	{
		//assigned
		BooleanLiteral reason_literal = (*explanation)[i];
		BooleanVariable reason_variable = reason_literal.Variable();
//          state_.update_vars_analyzer_.insert(reason_variable.index_);

          //ignore variables at level 0 -> these are unit clauses (in future need to take care, these might be assumptions)
		if (state_.assignments_.GetAssignmentLevel(reason_variable) == 0) { continue; }

		//ignore variable if it was already processed
		if (seen_.IsPresent(reason_variable.index_)) { continue; }

		//label it as seen so we do not process the same variable twice in the future for the current conflict
		seen_.Insert(reason_variable.index_);

		//experimental for now, might remove
		if (internal_parameters_.bump_decision_variables == true || state_.assignments_.GetAssignmentPropagator(reason_variable) != NULL)
		{
			state_.variable_selector_.BumpActivity(reason_variable); //not sure about this one, but I am leaving it for now
		}

		//either classify the literal as part of a learned clause (if its decision level is smaller than the current level)
		//or as a candidate for later analysis (if its level is the same as the current level)
		//updates backtrack level and glucose variable candidates appropriately
		int literal_decision_level = state_.assignments_.GetAssignmentLevel(reason_variable);

		if (literal_decision_level == state_.GetCurrentDecisionLevel())
		{
			num_current_decision_level_literals_++;
			if (use_glucose_bumping_) { glucose_bumper_candidates_.push_back(reason_variable); }
		}
		else
		{
			learned_clause_literals_.push_back(~reason_literal);
			backtrack_level_ = std::max(backtrack_level_, literal_decision_level);
		}
	}
	runtime_assert(num_current_decision_level_literals_ >= 1); //there has to be at least one literal from the current decision level responsible for the failure
}

PropagatorGeneric * ConstraintSatisfactionSolver::ProcessConflictAnalysisResult(ConflictAnalysisResultClausal& result)
{

//        for (BooleanLiteral l : result.learned_clause_literals) {
//          state_.update_vars_conflict_clause_.insert(l.VariableIndex());
//        }

	//unit clauses are treated in a special way: they are added as decision literals at decision level 0. This might change in the future if a better idea presents itself
	if (result.learned_clause_literals.size() == 1)
	{
		counters_.unit_clauses_learned++;

		state_.Backtrack(0);
		state_.EnqueueDecisionLiteral(result.propagated_literal);
		state_.UpdateMovingAveragesForRestarts(1);
	}
	else
	{
		int lbd = TwoWatchedClause::computeLBD(result.learned_clause_literals, state_) - 1; //minus one since the 1UP will change its decision level
		state_.UpdateMovingAveragesForRestarts(lbd);

		TwoWatchedClause* learned_clause = state_.AddLearnedClauseToDatabase(result.learned_clause_literals);

		state_.Backtrack(result.backtrack_level); //todo refactor so that the propagator directly does the assignment, perhaps with the new interface for adding clauses
		state_.EnqueuePropagatedLiteral(result.propagated_literal, &state_.propagator_clausal_, reinterpret_cast<uint64_t>(learned_clause)); //todo can this be done cleaner, without having to refer to how 'code' is interpreted by the propagator?

		runtime_assert(lbd == TwoWatchedClause::computeLBD(result.learned_clause_literals, state_));
	}
	return state_.PropagateEnqueued();
}

//maybe return a core rather than changing the analysis result for design reasons
std::vector<BooleanLiteral> ConstraintSatisfactionSolver::RefineConflictAnalysisResultIntoUnsatisfiableCore(ConflictAnalysisResultClausal &analysis_result)
{
	runtime_assert(1 == 2); //need to check this code

	static DirectlyHashedIntegerSet already_processed(0);
	already_processed.Resize(state_.GetNumberOfVariables()+1);
	already_processed.Clear();

	learned_clause_minimiser_.RemoveImplicationGraphDominatedLiterals(analysis_result); //note that this is redundant if clause minimisation is enabled. The idea is that for cores we always minimise regardless of whether we normally use clause minimisation

	//we may remove the propagated literal from the learned clause since it is implied by the other literals by definition
	auto prop_lit_iter = std::find(analysis_result.learned_clause_literals.begin(), analysis_result.learned_clause_literals.end(), analysis_result.propagated_literal);
	analysis_result.learned_clause_literals.erase(prop_lit_iter);

	//at this point, we know the learned clause contains only assumptions, but some assumptions might be implied
	//we now expand implied assumptions until all literals are decision assumptions
	std::vector<BooleanLiteral> core;
	std::vector<BooleanLiteral> unconsidered_literals;
	for (BooleanLiteral lit : analysis_result.learned_clause_literals) { unconsidered_literals.push_back(~lit); }

	while (!unconsidered_literals.empty())
	{
		BooleanLiteral current_literal = unconsidered_literals.back();
		unconsidered_literals.pop_back();
		//runtime_assert(state_.assignments_.IsAssumptionAndAssigned(current_literal));

		if (state_.assignments_.IsDecision(current_literal))
		{
			core.push_back(current_literal);
		}
		else
		{//we now expand the reason for the implied assumption with other assumptions
			PropagatorGeneric * propagator = state_.assignments_.GetAssignmentPropagator(current_literal);
			ExplanationGeneric * explanation = propagator->ExplainLiteralPropagation(current_literal, state_);
			for (int i = 0; i < explanation->Size(); i++)
			{
				BooleanLiteral reason_literal = (*explanation)[i];
				runtime_assert(state_.assignments_.IsAssignedTrue(reason_literal));
				//ignore literals already processed: this may happen if an assumption was responsible for two or more other implied assumptions
				if (already_processed.IsPresent(reason_literal.VariableIndex()) || state_.assignments_.GetAssignmentLevel(reason_literal) == 0) { continue; }
				unconsidered_literals.push_back(reason_literal);
				already_processed.Insert(reason_literal.VariableIndex());
			}
		}
	}
	//at this point, all literals in the core are non-implied assumptions
	already_processed.Clear();
	return core;
}

BooleanLiteral ConstraintSatisfactionSolver::FindNextReasonLiteralOnTheTrail()
{

        assert(seen_.GetCapacity() == state_.GetNumberOfVariables() + 1);
	runtime_assert(num_current_decision_level_literals_ > 0);
	//expand a node of the current decision level
	//find a literal that you have already seen
	//the ones you have not seen are not important for this conflict
	BooleanLiteral next_literal;
	do
	{
		next_literal = state_.GetLiteralFromTheBackOfTheTrail(num_trail_literals_examined_);
		assert(state_.assignments_.GetAssignmentLevel(next_literal.Variable()) == state_.GetCurrentDecisionLevel());
		num_trail_literals_examined_++;
	} while (seen_.IsPresent(next_literal.VariableIndex()) == false);

	num_current_decision_level_literals_--; //we are expanding a node and therefore reducing the number of literals of the current level that are left to be considered
	return next_literal;
}

void ConstraintSatisfactionSolver::PerformGlucoseVariableBumping()
{
	runtime_assert(1 == 2); //disabled for now, should enable it! Mainly the problem is when I introduced propagators, conflict clauses are not directly accessible
	/*
	int lbd = TwoWatchedClause::computeLBD(learned_clause_literals_, state_) - 1;
	//	std::cout << glucose_bumper_candidates.Size() << " " << lbd << std::endl;
	for (BooleanVariable var : glucose_bumper_candidates_)
	{
		if (state_.assignments_.GetAssignmentLevel(var) == state_.GetCurrentDecisionLevel())
		{
			if (state_.assignments_.GetAssignmentPropagator(var) != NULL && state_.assignments_.GetAssignmentPropagator(var)->best_literal_blocking_distance_ < lbd)
			{
				state_.variable_selector_.BumpActivity(var);
			}
		}
	}*/
}

void ConstraintSatisfactionSolver::ClearConflictAnalysisDataStructures()
{
	//for (BooleanLiteral lit : learned_clause_literals_) { seen_[lit.Variable().index_] = false; } //if I add glucose bumping permanently I can use those literals for clearing rather than clearing in the loop above - might make the code simpler - this is because the literals from the current decision level are already clearned, and only glucose ones are not yet cleared
	seen_.Clear(); //switched to a simple class for managing seen, todo remove the remains of the old code
	learned_clause_literals_.clear();
	glucose_bumper_candidates_.clear();
	backtrack_level_ = 0;
	num_current_decision_level_literals_ = 0; //this is anyway set to zero after conflict analysis
	num_trail_literals_examined_ = 0;
}

bool ConstraintSatisfactionSolver::ShouldRestart()
{
	//todo encapsulate the restart strategies within separate classes?

	//if already at the root, do not restart. Note that this affects the cleanup of learned clauses, but for now we ignore this
	if (state_.GetCurrentDecisionLevel() == 0) { return false; }
	//do not consider restarting if the minimum number of conflicts did not occur
	if (counters_.conflicts_until_restart > 0) { return false; }

	if (internal_parameters_.restart_strategy_ == InternalParameters::RestartStrategy::LUBY
			|| internal_parameters_.restart_strategy_ == InternalParameters::RestartStrategy::CONSTANT)
	{
		return true;
	}

	//using the glucose restart strategy as described in "Evaluating CDCL Restart Schemes" -> currently using the original version with simple moving averages instead of the exponential decays
	runtime_assert(internal_parameters_.restart_strategy_ == InternalParameters::RestartStrategy::GLUCOSE);

        if (counters_.conflicts_until_restart < -1000)
          return true;

	//should postpone the restart?
	if (counters_.conflicts >= 10000 && state_.GetNumberOfAssignedVariables() > 1.4*state_.simple_moving_average_block.GetCurrentValue())
	{
		counters_.blocked_restarts++;
		counters_.conflicts_until_restart = internal_parameters_.num_min_conflicts_per_restart;
		state_.simple_moving_average_lbd.Reset();
		return false;
	}

	//is the solver learning "bad" clauses?
	if (state_.simple_moving_average_lbd.GetCurrentValue()*0.8 > state_.cumulative_moving_average_lbd.GetCurrentValue())
	{
		state_.simple_moving_average_lbd.Reset();
		counters_.conflicts_until_restart = internal_parameters_.num_min_conflicts_per_restart;
		return true;
	} //TODO restart when encodings are scheduled
	else
	{
		return false;
	}
}

void ConstraintSatisfactionSolver::PerformRestartDuringSearch()
{
	if (state_.propagator_clausal_.clause_database_.TemporaryClausesExceedLimit())
	{//currently clause clean up can only be done at the root level, so we use this workaround when using assumptions. Todo fix.
		counters_.num_clause_cleanup++;
		state_.Backtrack(0);
		state_.propagator_clausal_.clause_database_.PromoteAndReduceTemporaryClauses();
	}
	else
	{
		int restart_level = assumptions_.size();
		if (restart_level < state_.GetCurrentDecisionLevel())
		{
			state_.Backtrack(assumptions_.size());
		}
	}

	counters_.num_restarts++;
        state_.num_restarts++;
        restarts_counter_debug++;

	if (internal_parameters_.restart_strategy_ == InternalParameters::RestartStrategy::LUBY)
	{
		counters_.conflicts_until_restart = (luby_generator_.GetNextElement() * internal_parameters_.restart_coefficient_);
	}
	else if (internal_parameters_.restart_strategy_ == InternalParameters::RestartStrategy::CONSTANT)
	{
		counters_.conflicts_until_restart = internal_parameters_.restart_coefficient_;
	}
	else if (internal_parameters_.restart_strategy_ == InternalParameters::RestartStrategy::GLUCOSE)
	{
		counters_.conflicts_until_restart = internal_parameters_.num_min_conflicts_per_restart;
	}
	else
	{
		runtime_assert(1 == 2);
	}

//        LogUsedVars();


        state_.AddScheduledEncodings();
        bool res = SetUnitClauses();
        assert(res);
}

void ConstraintSatisfactionSolver::PerformRestartToRoot()
{//todo - much of this method is copy-pasted from PerformRestartDuringSearch
	state_.Backtrack(0);

	if (state_.propagator_clausal_.clause_database_.TemporaryClausesExceedLimit())
	{
		counters_.num_clause_cleanup++;

		state_.propagator_clausal_.clause_database_.PromoteAndReduceTemporaryClauses();
	}

  state_.num_restarts++;
  counters_.num_restarts++;

	if (internal_parameters_.restart_strategy_ == InternalParameters::RestartStrategy::LUBY)
	{
		counters_.conflicts_until_restart = (luby_generator_.GetNextElement() * internal_parameters_.restart_coefficient_);
	}
	else if (internal_parameters_.restart_strategy_ == InternalParameters::RestartStrategy::CONSTANT)
	{
		counters_.conflicts_until_restart = internal_parameters_.restart_coefficient_;
	}
	else if (internal_parameters_.restart_strategy_ == InternalParameters::RestartStrategy::GLUCOSE)
	{
		counters_.conflicts_until_restart = internal_parameters_.num_min_conflicts_per_restart;
	}
	else
	{
		runtime_assert(1 == 2);
	}
}

void ConstraintSatisfactionSolver::CleanUp()
{
	if (state_.GetCurrentDecisionLevel() > 0) { PerformRestartToRoot(); }
}

void ConstraintSatisfactionSolver::UpdateConflictCounters()
{
	counters_.conflicts++;
	counters_.conflicts_until_restart--;
}

bool ConstraintSatisfactionSolver::SetUnitClauses()
{
	//TODO: should unit clauses actually be labelled as propagated at the root; rather than decisions at the root level?
	for (BooleanLiteral literal : state_.propagator_clausal_.clause_database_.unit_clauses_)
	{
		if (state_.assignments_.IsAssigned(literal))
		{
			assert(state_.assignments_.GetAssignment(literal.Variable()) == literal);
		}
		else
		{
			state_.EnqueueDecisionLiteral(literal);
		}
	}
	PropagatorGeneric* conflict_propagator = state_.PropagateEnqueued();
	return conflict_propagator == NULL;
}

SolverOutput ConstraintSatisfactionSolver::GenerateOutput()
{
	if (!state_.IsAssignmentBuilt())
	{//if no solution has been built but the search completed before the timeout, unsatisfiability has been proven
		return SolverOutput(stopwatch_.TimeElapsedInSeconds(), !stopwatch_.IsWithinTimeLimit(), std::vector<bool>(), -1, Conjunction());
	}
	else
	{//note that if a solution has been found, it must have been done within the time limit
		return SolverOutput(stopwatch_.TimeElapsedInSeconds(), false, state_.GetOutputAssignment(), 0, Conjunction());
	}
}

bool ConstraintSatisfactionSolver::CheckConflictAnalysisDataStructures()
{
	assert(IsSeenCleared()); //every element of seen_ is 'false'
	assert(learned_clause_literals_.empty());
	assert(glucose_bumper_candidates_.empty());
	assert(backtrack_level_ == 0);
	assert(num_current_decision_level_literals_ == 0);
	assert(num_trail_literals_examined_ == 0);
	return true;
}

bool ConstraintSatisfactionSolver::IsSeenCleared() const
{
	assert(seen_.GetNumPresentValues() == 0);
	//for (unsigned int i = 0; i < seen_.size(); i++) { assert(seen_[i] == false); }
	return true;
}
void ConstraintSatisfactionSolver::LogUsedVars() {
  return;

  simple_sat_solver::logger::Logger::Log2("PB Average depth: " + std::to_string(average_depth_));
  simple_sat_solver::logger::Logger::Log2("PB Lit count: " + std::to_string(literals_count_));
  simple_sat_solver::logger::Logger::Log2("PB Average number in conflict: " + std::to_string(average_number_));
  simple_sat_solver::logger::Logger::Log2("PB Number conflicts: " + std::to_string(constraint_count_));
  simple_sat_solver::logger::Logger::Log2("PB Average depth m2: " + std::to_string(average_depth_m2_));
  simple_sat_solver::logger::Logger::Log2("PB Lit count m2: " + std::to_string(literals_count_m2_));
  simple_sat_solver::logger::Logger::Log2("PB Average max distance: " + std::to_string(average_max_distance_));
  simple_sat_solver::logger::Logger::Log2("PB Average distance: " + std::to_string(average_distance_));

  std::string depth_count;
  for (int i = 0; i < depth_count_.size(); i++) {
    depth_count += " " + std::to_string(depth_count_[i]);
  }
  simple_sat_solver::logger::Logger::Log2("PB depth counts:" + depth_count);

//  std::string analyze_vars;
//  for (int i : state_.update_vars_analyzer_) {
//    analyze_vars += " " + std::to_string(i);
//  }
//  simple_sat_solver::logger::Logger::Log2("Analyzed conflict vars:" + analyze_vars);
//
//
//  std::string clause_vars;
//  for (int i : state_.update_vars_conflict_clause_) {
//    clause_vars += " " + std::to_string(i);
//  }
//  simple_sat_solver::logger::Logger::Log2("Learned clause vars:" + clause_vars);
//
//
//  state_.update_vars_conflict_clause_.clear();
//  state_.update_vars_analyzer_.clear();

}
void ConstraintSatisfactionSolver::LogLearnedClause(
    ConflictAnalysisResultClausal analysis_result) {
  PropagatorPseudoBoolean2 * prop = nullptr;
  for (auto p : state_.additional_propagators_) {
    if(PropagatorPseudoBoolean2 * v = dynamic_cast<PropagatorPseudoBoolean2*>(p)) {
      prop = v;
      break;
    }
  }
  if (prop == nullptr)
    return;

  for (WatchedPseudoBooleanConstraint2 * c : prop->pseudo_boolean_database_.permanent_constraints_) {
    if (GeneralizedTotaliser *encoder =
            dynamic_cast<GeneralizedTotaliser*>(c->encoder_)) {

      std::vector<BooleanLiteral> found_lits;
      std::vector<BooleanLiteral> propagated_lits;
      for (BooleanLiteral l : analysis_result.learned_clause_literals) {
        if (encoder->IsEncoded(l)) {
          found_lits.push_back(l);
        } else if (c->unencoded_constraint_literals_.count(l) > 0 || c->unencoded_constraint_literals_.count(~l) > 0) {
          propagated_lits.push_back(l);
        }
      }

      for (BooleanLiteral l : found_lits) {
        int depth = encoder->GetDepth(l);
        while (depth >= depth_count_.size())
          depth_count_.push_back(0);
        depth_count_[depth] ++;
      }

      constraint_count_++;
      average_number_ = average_number_ * (constraint_count_ - 1.) / constraint_count_;
      average_number_ += ((double)found_lits.size())/ constraint_count_;
      for (BooleanLiteral l : found_lits) {
        literals_count_++;
        double depth = encoder->GetDepth(l);
        average_depth_ = (average_depth_ * (literals_count_ - 1.)) / (literals_count_);
        average_depth_ += depth / literals_count_;
      }
      if (found_lits.size() >= 2) {
        for (BooleanLiteral l : found_lits) {
          literals_count_m2_++;
          double depth = encoder->GetDepth(l);
          average_depth_m2_ =
              (average_depth_ * (literals_count_m2_ - 1.)) / (literals_count_m2_);
          average_depth_m2_ += depth / literals_count_m2_;
        }

        int max_distance = 0;
        for (int i = 0; i < found_lits.size(); ++i) {
          for (int j =i + 1 ; j < found_lits.size(); ++j) {
            BooleanLiteral l1 = found_lits[i];
            BooleanLiteral l2 = found_lits[j];
            int distance = encoder->Distance(l1,l2);
            if (distance > max_distance)
              max_distance = distance;

            average_distance_count_++;
            average_distance_ = average_distance_ * (average_distance_count_ - 1.) / average_distance_count_;
            average_distance_ += (double(distance) / average_distance_count_);
          }
        }
        max_distance_count++;
        average_max_distance_ = (average_max_distance_ * (max_distance_count - 1.)) / max_distance_count;
        average_max_distance_ += double(max_distance) / max_distance_count;
      }
    }
  }


}

} //end Pumpkin namespace