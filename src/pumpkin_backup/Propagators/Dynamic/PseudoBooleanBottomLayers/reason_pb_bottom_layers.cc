//
// Created by jens on 18-05-21.
//

#include "reason_pb_bottom_layers.h"


namespace Pumpkin {

ReasonPbBottomLayers::ReasonPbBottomLayers(
    WatchedPbBottomLayersConstraint *constraint, SolverState &state)
    : scaling_factor_(1) {
  explanation_ = new ExplanationPbBottomLayers(constraint, state);
}
ReasonPbBottomLayers::ReasonPbBottomLayers(
    WatchedPbBottomLayersConstraint *constraint, BooleanLiteral propagated_literal,
    SolverState &state)
    : scaling_factor_(1) {
  explanation_ = new ExplanationPbBottomLayers(constraint, state,
                                                         propagated_literal);
}
void ReasonPbBottomLayers::RoundToOne(BooleanLiteral resolving_literal,
                                                SolverState &state) {
}
Term ReasonPbBottomLayers::operator[](size_t index) {
  BooleanLiteral l = (*explanation_)[index];
  return Term(l, scaling_factor_);
}
uint64_t ReasonPbBottomLayers::GetCoefficient(BooleanLiteral literal) {
  // TODO check if literals is in explanation
  return scaling_factor_;
}
void ReasonPbBottomLayers::MultiplyByFraction(uint64_t numerator,
                                                        uint64_t denominator) {
  assert(denominator == 1);
  scaling_factor_ = numerator;
}
uint32_t ReasonPbBottomLayers::GetRightHandSide() { return 1;}
size_t ReasonPbBottomLayers::Size() { return explanation_->Size(); }
}
