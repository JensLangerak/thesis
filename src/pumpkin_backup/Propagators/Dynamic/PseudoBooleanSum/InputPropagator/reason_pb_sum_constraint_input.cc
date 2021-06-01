//
// Created by jens on 16-03-21.
//

#include "reason_pb_sum_constraint_input.h"

namespace Pumpkin {

ReasonPbSumConstraintInput::ReasonPbSumConstraintInput(
    WatchedPbSumConstraint *constraint, SolverState &state)
    : scaling_factor_(1) {
  explanation_ = new ExplanationPbSumConstraintInput(constraint, state);
}
ReasonPbSumConstraintInput::ReasonPbSumConstraintInput(
    WatchedPbSumConstraint *constraint, BooleanLiteral propagated_literal,
    SolverState &state)
    : scaling_factor_(1) {
  explanation_ = new ExplanationPbSumConstraintInput(constraint, state,
                                              propagated_literal);
}
void ReasonPbSumConstraintInput::RoundToOne(BooleanLiteral resolving_literal,
                                     SolverState &state) {
}
Term ReasonPbSumConstraintInput::operator[](size_t index) {
  BooleanLiteral l = (*explanation_)[index];
  return Term(l, scaling_factor_);
}
uint64_t ReasonPbSumConstraintInput::GetCoefficient(BooleanLiteral literal) {
  // TODO check if literals is in explanation
  return scaling_factor_;
}
void ReasonPbSumConstraintInput::MultiplyByFraction(uint64_t numerator,
                                             uint64_t denominator) {
  assert(denominator == 1);
  scaling_factor_ = numerator;
}
uint32_t ReasonPbSumConstraintInput::GetRightHandSide() { return 1;}
size_t ReasonPbSumConstraintInput::Size() { return explanation_->Size(); }
}