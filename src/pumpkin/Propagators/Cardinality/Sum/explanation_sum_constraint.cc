//
// Created by jens on 17-10-20.
//

#include "explanation_sum_constraint.h"
namespace Pumpkin {

BooleanLiteral ExplanationSumConstraint::operator[](int index) const {
  assert(index < lits_.size());
  return lits_[index];
}
BooleanLiteral ExplanationSumConstraint::operator[](size_t index) const {
  assert(index < lits_.size());
  return lits_[index];
}
ExplanationSumConstraint::ExplanationSumConstraint(
    const WatchedSumConstraint *constraint, SolverState &state) {
  assert(constraint!= nullptr);
  lits_ = std::vector<BooleanLiteral>();
  int nr_lits = 0;
    for (nr_lits = 0; nr_lits < constraint->outputs_.size(); ++nr_lits) {
    BooleanLiteral l = constraint->outputs_[nr_lits];
    if (state.assignments_.IsAssignedFalse(l)) {
      lits_.push_back(~l); // TODO is this correct?
      break;
    }
  }
    assert(nr_lits <= constraint->true_order_.size());
    for (int i = 0; i <= nr_lits; ++i) {
      lits_.push_back(constraint->true_order_[i]);
    }
}
ExplanationSumConstraint::ExplanationSumConstraint(
    const WatchedSumConstraint *constraint, SolverState &state,
    BooleanLiteral propagated_literal) {

  assert(constraint!= nullptr);
  lits_ = std::vector<BooleanLiteral>();
  int nr_lits = 0;
  bool found = false;
  for (nr_lits = 0; nr_lits < constraint->outputs_.size(); ++nr_lits) {
    BooleanLiteral l = constraint->outputs_[nr_lits];
      lits_.push_back(constraint->true_order_[nr_lits]);
    if (l == propagated_literal) {
      found = true;
      break;
    }
  }
  assert(found);
}
size_t ExplanationSumConstraint::Size() const { return lits_.size(); }
} // namespace Pumpkin