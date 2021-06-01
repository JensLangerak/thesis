//
// Created by jens on 17-10-20.
//

#include "reason_sum_constraint.h"

namespace Pumpkin {

ReasonSumConstraint::ReasonSumConstraint(
    WatchedSumConstraint *constraint, SolverState &state)
    : scaling_factor_(1) {
  explanation_ = new ExplanationSumConstraint(constraint, state);
}
ReasonSumConstraint::ReasonSumConstraint(
    WatchedSumConstraint *constraint, BooleanLiteral propagated_literal,
    SolverState &state)
    : scaling_factor_(1) {
  explanation_ = new ExplanationSumConstraint(constraint, state,
                                                      propagated_literal);
}
void ReasonSumConstraint::RoundToOne(BooleanLiteral resolving_literal,
                                             SolverState &state) {
  }
Term ReasonSumConstraint::operator[](size_t index) {
  BooleanLiteral l = (*explanation_)[index];
  return Term(l, scaling_factor_);
}
uint64_t ReasonSumConstraint::GetCoefficient(BooleanLiteral literal) {
  // TODO check if literals is in explanation
  return scaling_factor_;
}
void ReasonSumConstraint::MultiplyByFraction(uint64_t numerator,
                                                     uint64_t denominator) {
  assert(denominator == 1);
  scaling_factor_ = numerator;
}
uint32_t ReasonSumConstraint::GetRightHandSide() { return 1;}
size_t ReasonSumConstraint::Size() { return explanation_->Size(); }
}
