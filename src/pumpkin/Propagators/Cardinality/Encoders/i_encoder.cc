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
bool IEncoder::IsAdded(BooleanLiteral l) { return EncodingAdded();}
void IEncoder::DebugInfo(SolverState &state) {}

IEncoder *IEncoder::IFactory::Create(WatchedCardinalityConstraint &constraint) {
  return Create(constraint.literals_, constraint.min_, constraint.max_);
}
IEncoder *IEncoder::IFactory::Create(CardinalityConstraint &constraint) {
  return Create(constraint.literals, constraint.min, constraint.max);
}
IEncoder *IEncoder::IFactory::Create(std::vector<BooleanLiteral> variables,
                                     int min, int max) {
  IEncoder * encoder = CallConstructor(variables, min, max);
  encoder->add_dynamic_ = this->add_dynamic_;
  encoder->add_incremental = this->add_incremetal_;
  encoder->add_delay = this->add_delay_;
  return encoder;
}
IEncoder *IEncoder::IFactory::Create(SumConstraint &constraint) {
  IEncoder * encoder = Create(constraint.input_literals, 0, constraint.output_literals.size());
  encoder->SetSumLiterals(constraint.output_literals);
  return encoder;
}
}