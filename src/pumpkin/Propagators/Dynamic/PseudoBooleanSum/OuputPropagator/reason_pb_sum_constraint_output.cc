//
// Created by jens on 16-03-21.
//

#include "reason_pb_sum_constraint_output.h"

namespace Pumpkin {

ReasonPbSumConstraintOutput::ReasonPbSumConstraintOutput(
    WatchedPbSumConstraint *constraint, SolverState &state)
    : scaling_factor_(1) {
  explanation_ = new ExplanationPbSumConstraintOutput(constraint, state);
}
ReasonPbSumConstraintOutput::ReasonPbSumConstraintOutput(
    WatchedPbSumConstraint *constraint, BooleanLiteral propagated_literal,
    SolverState &state)
    : scaling_factor_(1) {
  explanation_ = new ExplanationPbSumConstraintOutput(constraint, state,
                                              propagated_literal);
}
void ReasonPbSumConstraintOutput::RoundToOne(BooleanLiteral resolving_literal,
                                     SolverState &state) {
}
Term ReasonPbSumConstraintOutput::operator[](size_t index) {
  BooleanLiteral l = (*explanation_)[index];
  return Term(l, scaling_factor_);
}
uint64_t ReasonPbSumConstraintOutput::GetCoefficient(BooleanLiteral literal) {
  // TODO check if literals is in explanation
  return scaling_factor_;
}
void ReasonPbSumConstraintOutput::MultiplyByFraction(uint64_t numerator,
                                             uint64_t denominator) {
  assert(denominator == 1);
  scaling_factor_ = numerator;
}
uint32_t ReasonPbSumConstraintOutput::GetRightHandSide() { return 1;}
size_t ReasonPbSumConstraintOutput::Size() { return explanation_->Size(); }
}