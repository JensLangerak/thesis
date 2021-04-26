//
// Created by jens on 16-03-21.
//

#include "explanation_pb_sum_constraint_input.h"

namespace Pumpkin {

BooleanLiteral ExplanationPbSumConstraintInput::operator[](int index) const {
  assert(index < lits_.size());
  return lits_[index];
}
BooleanLiteral ExplanationPbSumConstraintInput::operator[](size_t index) const {
  assert(index < lits_.size());
  return lits_[index];
}
ExplanationPbSumConstraintInput::ExplanationPbSumConstraintInput(
    WatchedPbSumConstraint *constraint, SolverState &state) {
  assert(constraint != nullptr);
  //  WeightedLiteral wl;
  lits_ = std::vector<BooleanLiteral>();
  for (BooleanLiteral l : constraint->set_literals_) {
    lits_.push_back(l);
  }
  bool found = false;
  if (constraint->true_count_ <= constraint->max_) {
    for (WeightedLiteral o : constraint->outputs_) {
      if (o.weight <= constraint->true_count_ &&
          state.assignments_.IsAssignedFalse(o.literal)) {
        lits_.push_back(~o.literal);
        found = true;
        break;
      }
    }
    assert(found);
  }
  constraint->UpdateCounts(lits_, state);
}
ExplanationPbSumConstraintInput::ExplanationPbSumConstraintInput(
    WatchedPbSumConstraint *constraint, SolverState &state,
    BooleanLiteral propagated_literal) {
  for (BooleanLiteral l : constraint->set_literals_) {
    lits_.push_back(l);
  }
  if (!constraint->set_outputs_.empty()) {
    lits_.push_back(~(constraint->set_outputs_.back().literal));
  }
  constraint->UpdateCounts(lits_, state);
}
size_t ExplanationPbSumConstraintInput::Size() const { return lits_.size(); }

} // namespace Pumpkin