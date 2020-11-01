//
// Created by jens on 28-10-20.
//
#include "i_encoder.h"
#include "../../../Engine/solver_state.h"
namespace Pumpkin {
IEncoder::~IEncoder() {}
IEncoder::IFactory::~IFactory() {}
std::vector<std::vector<BooleanLiteral>>
IEncoder::Encode(SolverState &state, std::vector<BooleanLiteral> lits) {
    return Encode(state);
}
IEncoder *IEncoder::IFactory::Create(WatchedCardinalityConstraint &constraint) {
  return Create(constraint.literals_, constraint.min_, constraint.max_);
}
IEncoder *IEncoder::IFactory::Create(CardinalityConstraint &constraint) {
  return Create(constraint.literals, constraint.min, constraint.max);
}
IEncoder *IEncoder::IFactory::Create(std::vector<BooleanLiteral> variables,
                                     int min, int max) {
  IEncoder * encoder = CallConstructor(variables, min, max);
  encoder->add_dynamic_ = add_dynamic_;
  return encoder;
}
}