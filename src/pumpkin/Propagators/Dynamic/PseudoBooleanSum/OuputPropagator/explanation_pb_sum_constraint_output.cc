//
// Created by jens on 16-03-21.
//

#include "explanation_pb_sum_constraint_output.h"

namespace Pumpkin {

BooleanLiteral ExplanationPbSumConstraintOutput::operator[](int index) const {
  assert(index < lits_.size());
  return lits_[index];
}
BooleanLiteral ExplanationPbSumConstraintOutput::operator[](size_t index) const {
  assert(index < lits_.size());
  return lits_[index];
}
ExplanationPbSumConstraintOutput::ExplanationPbSumConstraintOutput(
    WatchedPbSumConstraint *constraint, SolverState &state) {
  assert(constraint != nullptr);
  lits_ = std::vector<BooleanLiteral>();
  assert(!constraint->set_outputs_.empty());
  WeightedLiteral output = constraint->set_outputs_.back();
  assert(constraint->true_count_ >= output.weight);
  assert(state.assignments_.IsAssignedFalse(output.literal));
  lits_.push_back(~(output.literal));
  int sum =0;
  for (BooleanLiteral l : constraint->set_literals_) {
    int i = constraint->lit_to_input_index_[l.ToPositiveInteger()];
    int w = constraint->inputs_[i].weight;
    assert(constraint->inputs_[i].literal == l);
    sum += w;
    lits_.push_back(l);
    if (sum >= output.weight)
      break;
  }

  constraint->UpdateCounts(lits_, state);
}
ExplanationPbSumConstraintOutput::ExplanationPbSumConstraintOutput(
    WatchedPbSumConstraint *constraint, SolverState &state,
    BooleanLiteral propagated_literal) {
  BooleanLiteral input = ~propagated_literal;
  assert(constraint->lit_to_input_index_.count(input.ToPositiveInteger()) > 0);
  int pos = state.assignments_.GetTrailPosition(propagated_literal.Variable());
  WeightedLiteral set_output;
  assert(constraint->outputs_.size() > 0);
  bool found = false;
  for (int i = constraint->set_outputs_.size() - 1; i >= 0; --i) {
    set_output = constraint->set_outputs_[i];
    if (state.assignments_.GetTrailPosition(set_output.literal.Variable()) < pos) {
      found = true;
      break;
    }
  }
  assert(found);
  assert(state.assignments_.IsAssignedFalse(set_output.literal));
  lits_.push_back(~(set_output.literal));
  int index = constraint->lit_to_input_index_[input.ToPositiveInteger()];
  WeightedLiteral il = constraint->inputs_[index];
  int target_sum = set_output.weight - il.weight;
  for (auto wl : constraint->set_literals_) {
    int i = constraint->lit_to_input_index_[wl.ToPositiveInteger()];
    WeightedLiteral l = constraint->inputs_[i];
    target_sum -= l.weight;
    lits_.push_back(wl);
    if (target_sum <= 0)
      break;
  }
  assert(target_sum <= 0);

  constraint->UpdateCounts(lits_, state);
}
size_t ExplanationPbSumConstraintOutput::Size() const { return lits_.size(); }

} // namespace Pumpkin