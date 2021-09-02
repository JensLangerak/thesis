//
// Created by jens on 10-12-20.
//

#include "reason_pseudo_boolean_constraint.h"

namespace Pumpkin {

ReasonPseudoBooleanConstraint2::ReasonPseudoBooleanConstraint2(
    WatchedPseudoBooleanConstraint2 *constraint, SolverState &state)
    : scaling_factor_(1) {
  explanation_ = new ExplanationPseudoBooleanConstraint2(constraint, state);
}
ReasonPseudoBooleanConstraint2::ReasonPseudoBooleanConstraint2(
    WatchedPseudoBooleanConstraint2 *constraint, BooleanLiteral propagated_literal,
    SolverState &state)
    : scaling_factor_(1) {
  explanation_ = new ExplanationPseudoBooleanConstraint2(constraint, state,
                                                      propagated_literal);
}
void ReasonPseudoBooleanConstraint2::RoundToOne(BooleanLiteral resolving_literal,
                                             SolverState &state) {
}
Term ReasonPseudoBooleanConstraint2::operator[](size_t index) {
  BooleanLiteral l = (*explanation_)[index];
  return Term(l, scaling_factor_);
}
uint64_t ReasonPseudoBooleanConstraint2::GetCoefficient(BooleanLiteral literal) {
  // TODO check if literals is in explanation
  return scaling_factor_;
}
void ReasonPseudoBooleanConstraint2::MultiplyByFraction(uint64_t numerator,
                                                     uint64_t denominator) {
  assert(denominator == 1);
  scaling_factor_ = numerator;
}
uint32_t ReasonPseudoBooleanConstraint2::GetRightHandSide() { return 1;}
size_t ReasonPseudoBooleanConstraint2::Size() { return explanation_->Size(); }
}
