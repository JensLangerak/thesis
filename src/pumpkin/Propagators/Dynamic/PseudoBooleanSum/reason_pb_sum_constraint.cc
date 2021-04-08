//
// Created by jens on 16-03-21.
//

#include "reason_pb_sum_constraint.h"

namespace Pumpkin {

ReasonPbSumConstraint::ReasonPbSumConstraint(
    WatchedPbSumConstraint *constraint, SolverState &state)
    : scaling_factor_(1) {
  explanation_ = new ExplanationPbSumConstraint(constraint, state);
}
ReasonPbSumConstraint::ReasonPbSumConstraint(
    WatchedPbSumConstraint *constraint, BooleanLiteral propagated_literal,
    SolverState &state)
    : scaling_factor_(1) {
  explanation_ = new ExplanationPbSumConstraint(constraint, state,
                                              propagated_literal);
}
void ReasonPbSumConstraint::RoundToOne(BooleanLiteral resolving_literal,
                                     SolverState &state) {
}
Term ReasonPbSumConstraint::operator[](size_t index) {
  BooleanLiteral l = (*explanation_)[index];
  return Term(l, scaling_factor_);
}
uint64_t ReasonPbSumConstraint::GetCoefficient(BooleanLiteral literal) {
  // TODO check if literals is in explanation
  return scaling_factor_;
}
void ReasonPbSumConstraint::MultiplyByFraction(uint64_t numerator,
                                             uint64_t denominator) {
  assert(denominator == 1);
  scaling_factor_ = numerator;
}
uint32_t ReasonPbSumConstraint::GetRightHandSide() { return 1;}
size_t ReasonPbSumConstraint::Size() { return explanation_->Size(); }
}