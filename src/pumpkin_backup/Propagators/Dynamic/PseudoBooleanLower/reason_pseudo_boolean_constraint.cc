//
// Created by jens on 10-12-20.
//

#include "reason_pseudo_boolean_constraint.h"

namespace Pumpkin {

ReasonPseudoBooleanConstraint3::ReasonPseudoBooleanConstraint3(
    WatchedPseudoBooleanConstraint3 *constraint, SolverState &state)
    : scaling_factor_(1) {
  explanation_ = new ExplanationPseudoBooleanConstraint3(constraint, state);
}
ReasonPseudoBooleanConstraint3::ReasonPseudoBooleanConstraint3(
    WatchedPseudoBooleanConstraint3 *constraint, BooleanLiteral propagated_literal,
    SolverState &state)
    : scaling_factor_(1) {
  explanation_ = new ExplanationPseudoBooleanConstraint3(constraint, state,
                                                      propagated_literal);
}
void ReasonPseudoBooleanConstraint3::RoundToOne(BooleanLiteral resolving_literal,
                                             SolverState &state) {
}
Term ReasonPseudoBooleanConstraint3::operator[](size_t index) {
  BooleanLiteral l = (*explanation_)[index];
  return Term(l, scaling_factor_);
}
uint64_t ReasonPseudoBooleanConstraint3::GetCoefficient(BooleanLiteral literal) {
  // TODO check if literals is in explanation
  return scaling_factor_;
}
void ReasonPseudoBooleanConstraint3::MultiplyByFraction(uint64_t numerator,
                                                     uint64_t denominator) {
  assert(denominator == 1);
  scaling_factor_ = numerator;
}
uint32_t ReasonPseudoBooleanConstraint3::GetRightHandSide() { return 1;}
size_t ReasonPseudoBooleanConstraint3::Size() { return explanation_->Size(); }
}
