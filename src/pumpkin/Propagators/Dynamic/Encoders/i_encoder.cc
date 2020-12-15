//
// Created by jens on 28-10-20.
//
#include "i_encoder.h"

#include "../../../Basic Data Structures/problem_specification.h"
#include "../../../Engine/solver_state.h"
namespace Pumpkin {
template <class T>
IEncoder<T>::~IEncoder() {}

template <class T>
IEncoder<T>::IFactory::~IFactory() {}

template <class T>
std::vector<std::vector<BooleanLiteral>>
IEncoder<T>::Encode(SolverState &state, std::vector<BooleanLiteral> lits) {
    return Encode(state);
}

template <class T>
bool IEncoder<T>::IsAdded(BooleanLiteral l) { return EncodingAdded();}

template <class T>
void IEncoder<T>::DebugInfo(SolverState &state) {}

template <class T>
IEncoder<T> *IEncoder<T>::IFactory::Create(T &constraint) {
  IEncoder * encoder = CallConstructor(constraint);
  encoder->add_dynamic_ = this->add_dynamic_;
  encoder->add_incremental = this->add_incremetal_;
  encoder->add_delay = this->add_delay_;
  return encoder;
}

template class IEncoder<CardinalityConstraint>;
template class IEncoder<PseudoBooleanConstraint>;
template class IEncoder<SumConstraint>;
}