#include "solver_state.h"
#include "../../logger/logger.h"
#include "../Basic Data Structures/runtime_assert.h"

#include <algorithm>
#include <assert.h>
#include <iostream>

namespace Pumpkin {

SolverState::SolverState(int64_t num_Boolean_variables,
                         SolverParameters &params)
    : trail_(), variable_selector_(num_Boolean_variables),
      value_selector_(num_Boolean_variables),
      assignments_(
          num_Boolean_variables), // note that the 0th position is not used
      propagator_clausal_(num_Boolean_variables,
                          params.learned_clause_decay_factor),
      propagator_pseudo_boolean_(num_Boolean_variables),
      propagator_cardinality_(num_Boolean_variables),
      //      propagator_sum_(num_Boolean_variables),
      decision_level_(0),
      simple_moving_average_lbd(params.glucose_queue_lbd_limit),
      simple_moving_average_block(params.glucose_queue_reset_limit) {
  propagator_clausal_.SetTrailIterator(trail_.begin());
  propagator_pseudo_boolean_.SetTrailIterator(trail_.begin());
  propagator_cardinality_.SetTrailIterator(trail_.begin());
  //  propagator_sum_.SetTrailIterator(trail_.begin());
}

void SolverState::EnqueueDecisionLiteral(BooleanLiteral decision_literal) {
  MakeAssignment(decision_literal, NULL, NULL);
}

bool SolverState::InsertPropagatedLiteral(
    BooleanLiteral propagated_literal,
    PropagatorGeneric *responsible_propagator, uint64_t code,
    int decision_level) {
  if (assignments_.IsAssigned(propagated_literal)) {
    assert(assignments_.GetAssignment(propagated_literal) == true);

  } else {
    MakeAssignment(propagated_literal, responsible_propagator, code,
                   decision_level);
    assert(propagator_cardinality_.cardinality_database_
               .watch_list_true[propagated_literal]
               .empty());
  }
  return true;
}

bool SolverState::EnqueuePropagatedLiteral(
    BooleanLiteral literal, PropagatorGeneric *responsible_propagator,
    uint64_t code) {
  assert(responsible_propagator != NULL);
  if (assignments_.IsAssigned(literal) ==
      true) // we are trying to propagate a variable that has already been
            // assigned a value
  {
    return assignments_.GetAssignment(literal.Variable()) ==
           literal; // return if the assignment is consistent with the
                    // propagation
  } else {
    MakeAssignment(literal, responsible_propagator, code);
    return true;
  }
}

PropagatorGeneric *SolverState::PropagateEnqueued() {
  size_t trail_before = trail_.size() - 1;

  while (propagator_clausal_.IsPropagationComplete(*this) == false ||
         propagator_pseudo_boolean_.IsPropagationComplete(*this) == false ||
         propagator_cardinality_.IsPropagationComplete(*this) == false) {
    //         propagator_sum_.IsPropagationComplete(*this) == false) {
    if (!propagator_clausal_.Propagate(*this)) {
      return &propagator_clausal_;
    }

    if (!propagator_pseudo_boolean_.PropagateOneLiteral(*this)) {
      return &propagator_pseudo_boolean_;
    }
    assert(propagator_cardinality_.CheckCounts(*this));
    if (!propagator_cardinality_.PropagateOneLiteral(*this)) {
      return &propagator_cardinality_;
    }
    assert(propagator_cardinality_.CheckCounts(*this));
    //    if (!propagator_sum_.PropagateOneLiteral(*this)) {
    //      return &propagator_sum_;
    //    }
  }
  return NULL;
}

void SolverState::IncreaseDecisionLevel() {
  decision_level_++;
  trail_delimiter_.push_back(trail_.end());

  //  trail_delimiter_.push_back(int(trail_.size()));
}

void SolverState::Backtrack(int backtrack_level) {
  runtime_assert(backtrack_level >= 0);
  runtime_assert(backtrack_level < decision_level_);
  int tet = decision_level_;
  while (decision_level_ != backtrack_level) {
    BacktrackOneLevel();
  }
  propagator_clausal_.Synchronise(*this);
  propagator_pseudo_boolean_.Synchronise(*this);
  propagator_cardinality_.Synchronise(*this);
  //  propagator_sum_.Synchronise(*this);
}

void SolverState::Reset() {
  // TODO fix bug
  propagator_cardinality_.SetTrailIterator(trail_.begin());
  //  propagator_sum_.SetTrailIterator(trail_.begin());
  if (GetCurrentDecisionLevel() != 0)
    Backtrack(0);
}

void SolverState::BacktrackOneLevel() {
  //  int num_assignments_for_removal =
  //      int(trail_.size() - trail_delimiter_.back());
  //  assert(num_assignments_for_removal >= 0);
  //  for (int i = 0; i < num_assignments_for_removal; i++) {
  while (trail_.back() != *trail_delimiter_.back()) {
    assert(assignments_.GetAssignmentLevel(trail_.back().Variable()) ==
           decision_level_);
    UndoLastAssignment();
  }
  assert(assignments_.GetAssignmentLevel(trail_.back().Variable()) ==
         decision_level_);
  UndoLastAssignment();
  trail_delimiter_.pop_back();
  decision_level_--;
  assert(trail_.size() == 0 ||
         assignments_.GetAssignmentLevel(trail_.back().Variable()) ==
             decision_level_);
}

void SolverState::UndoLastAssignment() {
  BooleanVariable last_assigned_variable = trail_.back().Variable();
  variable_selector_.Readd(last_assigned_variable);
  value_selector_.UpdatePolarity(
      last_assigned_variable,
      assignments_.IsAssignedTrue(last_assigned_variable));
  assignments_.UnassignVariable(last_assigned_variable);
  //  if (*trail_delimiter_.back() == trail_.back())
  //    trail_delimiter_.pop_back();
  trail_.pop_back();
}

BooleanVariable SolverState::GetHighestActivityUnassignedVariable() {
  BooleanVariable selected_variable =
      variable_selector_.PopHighestActivityVariable();
  while (selected_variable.IsUndefined() == false &&
         assignments_.IsAssigned(selected_variable) ==
             true) // iterate until you find the highest +unassigned+ variable -
                   // variable selection is done in a lazy fashion
  {
    selected_variable = variable_selector_.PopHighestActivityVariable();
  }
  if (selected_variable.IsUndefined() == false) {
    variable_selector_.Readd(
        selected_variable); // pop removes it, but we want to keep it inside the
                            // data structure in this call
  }
  return selected_variable;
}

BooleanLiteral SolverState::getLastDecisionLiteralOnTrail() const {
  return GetDecisionLiteralForLevel(GetCurrentDecisionLevel());
}

BooleanLiteral
SolverState::GetDecisionLiteralForLevel(int decision_level) const {
  runtime_assert(decision_level <= GetCurrentDecisionLevel());

  if (decision_level == 0) {
    return BooleanLiteral();
  } // return undefined literal when there are no decisions on the trail

  BooleanLiteral decision_literal = *trail_delimiter_[decision_level - 1];
  //      trail_[trail_delimiter_[decision_level - 1]];

  assert(assignments_.GetAssignmentPropagator(decision_literal.Variable()) ==
         NULL);
  assert(assignments_.GetAssignmentLevel(decision_literal.Variable()) ==
         decision_level);
  return decision_literal;
}

int SolverState::GetHighestDecisionLevelForLiterals(
    std::vector<BooleanLiteral> &literals) const {
  int backtrack_level = 0;
  for (BooleanLiteral literal : literals) {
    backtrack_level = std::max(
        backtrack_level, assignments_.GetAssignmentLevel(literal.Variable()));
  }
  return backtrack_level;
}

int SolverState::GetCurrentDecisionLevel() const { return decision_level_; }

size_t SolverState::GetNumberOfAssignedVariables() const {
  return int(trail_.size());
}

size_t SolverState::GetNumberOfVariables() const {
  return int(assignments_.GetNumberOfVariables());
}

BooleanLiteral SolverState::GetLiteralFromTrailAtPosition(size_t index) const {
  int t = trail_.size();
  assert(index < trail_.size());
  auto it = trail_.begin();
  for (size_t i = 0; i < index; ++i)
    it.Next();
  return *it;
}

BooleanLiteral
SolverState::GetLiteralFromTheBackOfTheTrail(size_t index) const {
  assert(index < trail_.size());
  TrailList<BooleanLiteral>::Iterator it = trail_.last();
  for (int i = 0; i < index; ++i)
    it.Previous();
  return *it;
  //  return trail_[trail_.size() - index - 1];
}

std::vector<bool> SolverState::GetOutputAssignment() const {
  std::vector<bool> output(GetNumberOfVariables() + 1);
  for (int i = 1; i <= GetNumberOfVariables(); i++) {
    runtime_assert(assignments_.IsAssigned(BooleanVariable(i)));
    output[i] = assignments_.GetAssignment(BooleanVariable(i)).IsPositive();
  }
  return output;
}

bool SolverState::IsAssignmentBuilt() {
  // check if there are any variables left that need to be assigned
  // if not, then the assignment is complete
  return GetHighestActivityUnassignedVariable().IsUndefined();
}

void SolverState::PrintTrail() const {
  std::cout << "Trail\n";
  for (int i = 0; i < GetNumberOfAssignedVariables(); i++) {
    std::cout << GetLiteralFromTheBackOfTheTrail(i).VariableIndex() << "\n";
  }
  std::cout << "end trail\n";
}

BooleanVariable SolverState::CreateNewVariable() {
  BooleanVariable new_variable(GetNumberOfVariables() + 1);

  variable_selector_.Grow();
  value_selector_.Grow();
  assignments_.Grow();
  propagator_clausal_.clause_database_.watch_list_.Grow();
  propagator_pseudo_boolean_.constraint_database_.watch_list_.Grow();
  //  propagator_cardinality_.cardinality_database_.watch_list_false.Grow();
  propagator_cardinality_.cardinality_database_.watch_list_true.Grow();
  //  propagator_sum_.sum_database_.watch_list_true.Grow();

  return new_variable;
}

void SolverState::CreateVariablesUpToIndex(int largest_variable_index) {
  for (int i = GetNumberOfVariables() + 1; i <= largest_variable_index; i++) {
    CreateNewVariable();
  }
}

void SolverState::AddUnitClause(BooleanLiteral &literal) {
  propagator_clausal_.clause_database_.AddUnitClause(literal);
}

void SolverState::AddUnitClauses(std::vector<BooleanLiteral> &units) {
  for (BooleanLiteral literal : units) {
    AddUnitClause(literal);
  }
}

void SolverState::AddClause(std::vector<BooleanLiteral> &literals) {
  propagator_clausal_.clause_database_.AddPermanentClause(literals, *this);
}

bool SolverState::AddUnitClauseDuringSearch(BooleanLiteral literal) {
  if (assignments_.IsAssigned(literal)) {
    return assignments_.IsAssignedTrue(literal);
  }

  if (GetCurrentDecisionLevel() != 0)
    Backtrack(0);
  EnqueueDecisionLiteral(literal);

  PropagatorGeneric *conflict_propagator = PropagateEnqueued();

  return conflict_propagator == NULL;
}

void SolverState::AddBinaryClause(BooleanLiteral a, BooleanLiteral b) {
  std::vector<BooleanLiteral> lits;
  lits.push_back(a);
  lits.push_back(b);
  AddClause(lits);
}

void SolverState::AddTernaryClause(BooleanLiteral a, BooleanLiteral b,
                                   BooleanLiteral c) {
  std::vector<BooleanLiteral> lits;
  lits.push_back(a);
  lits.push_back(b);
  lits.push_back(c);
  AddClause(lits);
}

void SolverState::AddImplication(BooleanLiteral a, BooleanLiteral b) {
  AddBinaryClause(~a, b);
}

TwoWatchedClause *
SolverState::AddLearnedClauseToDatabase(std::vector<BooleanLiteral> &literals) {
  TwoWatchedClause *learned_clause;
  if (TwoWatchedClause::computeLBD(literals, *this) <= 2) {
    // counters_.small_lbd_clauses_learned++;

    learned_clause =
        this->propagator_clausal_.clause_database_.AddPermanentClause(literals,
                                                                      *this);
  } else {
    learned_clause =
        this->propagator_clausal_.clause_database_.AddTemporaryClause(literals,
                                                                      *this);
    this->propagator_clausal_.clause_database_.BumpClauseActivity(
        learned_clause);

    // counters_.ternary_clauses_learned += (literals.size() == 3); //I don't
    // think I should keep the counter code here
  }
  return learned_clause;
}

void SolverState::UpdateMovingAveragesForRestarts(int learned_clause_lbd) {
  simple_moving_average_lbd.AddTerm(learned_clause_lbd);
  cumulative_moving_average_lbd.AddTerm(learned_clause_lbd);
  simple_moving_average_block.AddTerm(GetNumberOfAssignedVariables());
}

void SolverState::MakeAssignment(BooleanLiteral literal,
                                 PropagatorGeneric *responsible_propagator,
                                 uint64_t code, int decisionLevel) {
  assert(literal.IsUndefined() == false);
  assert(assignments_.IsAssigned(literal) == false);

  assignments_.MakeAssignment(literal.Variable(), literal.IsPositive(),
                              decisionLevel, responsible_propagator, code,
                              trail_.size());
  if (decisionLevel == decision_level_) {
    trail_.push_back(literal);
  } else {
    trail_.insert(trail_delimiter_[decisionLevel], literal);
  }
}
void SolverState::MakeAssignment(BooleanLiteral literal,
                                 PropagatorGeneric *responsible_propagator,
                                 uint64_t code) {
  assert(literal.IsUndefined() == false);
  assert(assignments_.IsAssigned(literal) == false);

  assignments_.MakeAssignment(literal.Variable(), literal.IsPositive(),
                              GetCurrentDecisionLevel(), responsible_propagator,
                              code, trail_.size());

  trail_.push_back(literal);
}
void SolverState::AddSumConstraint(SumConstraint &constraint) {
  // TODO
  //  propagator_sum_.sum_database_.AddPermanentConstraint(constraint, *this);
}
void SolverState::AddCardinality(CardinalityConstraint &constraint) {
  propagator_cardinality_.cardinality_database_.AddPermanentConstraint(
      constraint, *this);
}

TrailList<BooleanLiteral>::Iterator SolverState::GetTrailEnd() {
  return trail_.end();
}
TrailList<BooleanLiteral>::Iterator SolverState::GetTrailBegin() {
  return trail_.begin();
}
void SolverState::FullReset() {
  Reset();
  propagator_clausal_.SetTrailIterator(trail_.begin());
  propagator_cardinality_.SetTrailIterator(trail_.begin());
  propagator_cardinality_.ResetCounts();
  //  propagator_sum_.SetTrailIterator(trail_.begin());
  //  propagator_sum_.ResetCounts();
  propagator_pseudo_boolean_.SetTrailIterator(trail_.begin());
}
void SolverState::ResetPropagatorsToLevel() {
  TrailList<BooleanLiteral>::Iterator update = trail_.begin();
  if (decision_level_ > 0) {
    update = trail_delimiter_[decision_level_ - 1];
  }

  propagator_clausal_.SetTrailIterator(update);
  propagator_pseudo_boolean_.SetTrailIterator(update);
  propagator_cardinality_.SetTrailIterator(update);
  //  propagator_sum_.SetTrailIterator(update);
}

enum class LitValue { True, False, Unknown };
void SolverState::CheckClasualTrailState() {
  int t_size = trail_.size();
  auto stop = propagator_clausal_.next_position_on_trail_to_propagate_it;
  propagator_clausal_.next_position_on_trail_to_propagate_it = trail_.begin();
  while (propagator_clausal_.next_position_on_trail_to_propagate_it != stop) {
    bool res = propagator_clausal_.PropagateOneLiteral(*this);
    if (!res) {
      auto failed = propagator_clausal_.failure_clause_;
      std::vector<BooleanLiteral> lits;
      for (int i = 0; i < failed->literals_.size_; ++i)
        lits.push_back(failed->literals_[i]);
      std::vector<LitValue> values;
      std::vector<PropagatorGeneric *> propagator;
      for (BooleanLiteral l : lits) {
        if (assignments_.IsAssignedTrue(l))
          values.push_back(LitValue::True);
        else if (assignments_.IsAssignedFalse(l))
          values.push_back(LitValue::False);
        else
          values.push_back(LitValue::Unknown);
        if (assignments_.IsAssigned(l.Variable()))
          propagator.push_back(
              assignments_.GetAssignmentPropagator(l.Variable()));
        else
          propagator.push_back(nullptr);
      }

      assert(false);
    }
  }
  int t_size2 = trail_.size();
  assert(t_size == t_size2);
}

} // namespace Pumpkin