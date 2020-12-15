//
// Created by jens on 17-10-20.
//

#include "reason_cardinality_constraint.h"

namespace Pumpkin {

ReasonCardinalityConstraint::ReasonCardinalityConstraint(
    WatchedCardinalityConstraint *constraint, SolverState &state)
    : scaling_factor_(1) {
  explanation_ = new ExplanationCardinalityConstraint(constraint, state);
}
ReasonCardinalityConstraint::ReasonCardinalityConstraint(
    WatchedCardinalityConstraint *constraint, BooleanLiteral propagated_literal,
    SolverState &state)
    : scaling_factor_(1) {
  explanation_ = new ExplanationCardinalityConstraint(constraint, state,
                                                      propagated_literal);
}
void ReasonCardinalityConstraint::RoundToOne(BooleanLiteral resolving_literal,
                                             SolverState &state) {
  }
Term ReasonCardinalityConstraint::operator[](size_t index) {
  BooleanLiteral l = (*explanation_)[index];
  return Term(l, scaling_factor_);
}
uint64_t ReasonCardinalityConstraint::GetCoefficient(BooleanLiteral literal) {
  // TODO check if literals is in explanation
  return scaling_factor_;
}
void ReasonCardinalityConstraint::MultiplyByFraction(uint64_t numerator,
                                                     uint64_t denominator) {
  assert(denominator == 1);
  scaling_factor_ = numerator;
}
uint32_t ReasonCardinalityConstraint::GetRightHandSide() { return 1;}
size_t ReasonCardinalityConstraint::Size() { return explanation_->Size(); }
}
