//
// Created by jens on 16-10-20.
//

#include "propagator_cardinality.h"
#include "../../Engine/solver_state.h"
#include "reason_cardinality_constraint.h"
#include "watch_list_cardinality.h"
namespace Pumpkin {

PropagatorCardinality::PropagatorCardinality(int64_t num_variables)
    : cardinality_database_(num_variables), last_index_(0), last_propagated_(BooleanLiteral()) {}
bool PropagatorCardinality::PropagateLiteral(BooleanLiteral true_literal,
                                             SolverState &state) {
  assert(state.assignments_.IsAssignedTrue(true_literal));
  WatchListCardinality &watch_list_true = cardinality_database_.watch_list_true;
  std::vector<WatcherCardinalityConstraint> &watchers_true =
      watch_list_true[true_literal];
  size_t end_position = 0;
  size_t current_index = 0;
  if (!last_propagated_.IsUndefined() && last_propagated_ == true_literal)
    current_index = last_index_;
  for (; current_index < watchers_true.size();
       ++current_index) {
    WatchedCardinalityConstraint *constraint =
        watchers_true[current_index].constraint_;
    int true_count = 0;
    int false_count = 0;
    int unassinged_count = 0;
    for (BooleanLiteral l : constraint->literals_) {
      if (!state.assignments_.IsAssigned(l)) {
        ++unassinged_count;
      } else if (state.assignments_.IsAssignedTrue(l)){
        ++true_count;
      } else {
        ++false_count;
      }
    }
    //TODO handle with propagation
    constraint->true_count_ = true_count;
    constraint->false_count_ = false_count;

    watchers_true[end_position] = watchers_true[current_index];
    ++end_position;

    if (true_count == constraint->max_ || false_count == constraint->literals_.size() - constraint->min_) {
      for (BooleanLiteral l : constraint->literals_) {
        if (!state.assignments_.IsAssigned(l)) {
          uint64_t code = reinterpret_cast<uint64_t>(
              constraint); // the code will simply be a pointer to the propagating clause
          if (true_count == constraint->max_)
            state.EnqueuePropagatedLiteral(~l, this, code);
          else
            state.EnqueuePropagatedLiteral(l, this, code);
          return true;
        }
      }
    }

    if (true_count > constraint->max_ || false_count > constraint->literals_.size() - constraint->min_) {
      // restore remaining watchers
      for (size_t k = current_index + 1; k < watchers_true.size(); ++k) {
        watchers_true[end_position] = watchers_true[current_index];
        ++end_position;
      }
      watchers_true.resize(end_position);
      failure_constraint_ = constraint;
      return false;
    }


    last_index_ = current_index;
  }

  watchers_true.resize(end_position);
  return true;
}
ReasonGeneric *PropagatorCardinality::ReasonFailure(SolverState &state) {
  return new ReasonCardinalityConstraint(failure_constraint_, state);
}
ReasonGeneric *
PropagatorCardinality::ReasonLiteralPropagation(BooleanLiteral propagated_literal,
                                                SolverState &state) {
  assert(state.assignments_.GetAssignmentPropagator(propagated_literal.Variable()) == this);
  uint64_t code = state.assignments_.GetAssignmentCode(propagated_literal.Variable());
  WatchedCardinalityConstraint *propagating_constraint = reinterpret_cast<WatchedCardinalityConstraint*>(code);
  return new ReasonCardinalityConstraint(propagating_constraint, propagated_literal, state);
}
ExplanationGeneric *PropagatorCardinality::ExplainFailure(SolverState &state) {
  return failure_constraint_->ExplainFailure(state);
}
ExplanationGeneric *
PropagatorCardinality::ExplainLiteralPropagation(BooleanLiteral propagated_literal,
                                                 SolverState &state) {
  assert(state.assignments_.GetAssignmentPropagator(propagated_literal.Variable()) == this);
  uint64_t code = state.assignments_.GetAssignmentCode(propagated_literal.Variable());
  WatchedCardinalityConstraint *propagating_constraint = reinterpret_cast<WatchedCardinalityConstraint*>(code);
  return propagating_constraint->ExplainLiteralPropagation(propagated_literal, state);
}
void PropagatorCardinality::Synchronise(SolverState &state) {
  PropagatorGeneric::Synchronise(state);
  last_propagated_ = BooleanLiteral();
  last_index_ = 0;
}
} // namespace Pumpkin
