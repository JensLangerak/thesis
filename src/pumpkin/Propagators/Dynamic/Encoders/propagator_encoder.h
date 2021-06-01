//
// Created by jens on 31-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_PROPAGATOR_ENCODER_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_PROPAGATOR_ENCODER_H_

#include "../../../Utilities/boolean_literal.h"
#include "i_encoder.h"
#include <cassert>
#include <vector>
namespace Pumpkin {
template <class T>
class PropagatorEncoder : public IEncoder<T> {
public:
  void Encode(SolverState &state) override;
  void Encode(SolverState &state, std::vector<BooleanLiteral> lits) override;

  bool EncodingAddAtStart() override{ return false; };
  PropagatorEncoder() : IEncoder<T>() {};

  ~PropagatorEncoder() override;
  bool UpdateMax(int max, SolverState &state) override;
  class Factory : public IEncoder<T>::IFactory {
  public:
    Factory() : IEncoder<T>::IFactory(IEncoder<T>::NEVER, 1) {}

  protected:
    IEncoder<T> *CallConstructor(T &constraint) override {
      return new PropagatorEncoder();
    };
  };
};
} // namespace Pumpkin
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_PROPAGATOR_ENCODER_H_
