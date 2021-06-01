//
// Created by jens on 20-05-21.
//

#include "reason_dynamic_constraint.h"
namespace Pumpkin {
void ReasonDynamicConstraint::RoundToOne(BooleanLiteral resolving_literal,
                                                SolverState &state) {
}
Term ReasonDynamicConstraint::operator[](size_t index) {
  BooleanLiteral l = (*explanation_)[index];
  return Term(l, scaling_factor_);
}
uint64_t ReasonDynamicConstraint::GetCoefficient(BooleanLiteral literal) {
  // TODO check if literals is in explanation
  return scaling_factor_;
}
void ReasonDynamicConstraint::MultiplyByFraction(uint64_t numerator,
                                                        uint64_t denominator) {
  assert(denominator == 1);
  scaling_factor_ = numerator;
}
uint32_t ReasonDynamicConstraint::GetRightHandSide() { return 1;}
size_t ReasonDynamicConstraint::Size() { return explanation_->Size();

}
ReasonDynamicConstraint::~ReasonDynamicConstraint() {
  delete explanation_;
  explanation_ = nullptr;
}
ReasonDynamicConstraint::ReasonDynamicConstraint(
    ExplanationGeneric *explanation) : scaling_factor_(1), explanation_(explanation){}
}

