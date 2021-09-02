//
// Created by jens on 17-10-20.
//

#include "explanation_cardinality_constraint.h"
#include "../../../../logger/logger.h"
namespace Pumpkin {

BooleanLiteral ExplanationCardinalityConstraint::operator[](int index) const {
  assert(index < lits_.size());
  return lits_[index];
}
BooleanLiteral ExplanationCardinalityConstraint::operator[](size_t index) const {
  assert(index < lits_.size());
  return lits_[index];
}
ExplanationCardinalityConstraint::ExplanationCardinalityConstraint(
    WatchedCardinalityConstraint *constraint, SolverState &state) {
  assert(constraint!= nullptr);
  assert(constraint->true_count_ > constraint->max_ ||
         constraint->false_count_ >
             constraint->literals_.size() - constraint->min_);
  simple_sat_solver::logger::Logger::Log2("Explain Conflict: " + std::to_string(constraint->log_id_));
  lits_ = std::vector<BooleanLiteral>();
  // Check if the min or max constraint is violated
  bool select_value = constraint->true_count_ > constraint->max_;
  for (BooleanLiteral l : constraint->literals_) {
    if (state.assignments_.IsAssigned(l) &&
        state.assignments_.GetAssignment(l) == select_value)
      lits_.push_back(l);
  }

  constraint->UpdateCounts(lits_, state);


  int test = lits_.size();
  // TODO not sure if >= should be possible or that is should be ==
  assert(select_value && lits_.size() >= constraint->true_count_ ||
         (!select_value) && lits_.size() >= constraint->false_count_);
}
ExplanationCardinalityConstraint::ExplanationCardinalityConstraint(
    WatchedCardinalityConstraint *constraint, SolverState &state,
    BooleanLiteral propagated_literal) {
  assert(constraint!= nullptr);
  assert(state.assignments_.IsAssignedTrue(propagated_literal));
  simple_sat_solver::logger::Logger::Log2("Explain propagation: " + std::to_string(constraint->log_id_));
  // check if true for the minimum value is propagated or false for the upper bound.
  bool propagated_value = true;
  for (BooleanLiteral l : constraint->literals_) {
    if (l == propagated_literal || ~l == propagated_literal) {
      propagated_value = l == propagated_literal;
      break;
    }
  }

  assert((!propagated_value) && constraint->true_count_ >= constraint->max_ ||
         propagated_value && constraint->false_count_ >=
                                 constraint->literals_.size() - constraint->min_);
  // get the cause for the propagation
  bool cause_value = !propagated_value;
  int propagation_level = state.assignments_.GetTrailPosition(propagated_literal.Variable());
  lits_ = std::vector<BooleanLiteral>();
  for (BooleanLiteral l : constraint->literals_) {
    if (state.assignments_.IsAssigned(l) && state.assignments_.GetTrailPosition(l.Variable()) < propagation_level &&
        state.assignments_.GetAssignment(l) == cause_value)
      lits_.push_back(l);
  }
  constraint->UpdateCounts(lits_, state);
  // TODO trim lits if to many
}
size_t ExplanationCardinalityConstraint::Size() const { return lits_.size(); }
} // namespace Pumpkin