//
// Created by jens on 31-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_PROPAGATOR_ENCODER_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_PROPAGATOR_ENCODER_H_

#include "../../../Basic Data Structures/boolean_literal.h"
#include "i_encoder.h"
#include <cassert>
#include <vector>
namespace Pumpkin {
class PropagatorEncoder : public IEncoder {
public:
  std::vector<std::vector<BooleanLiteral>> Encode(SolverState &state) override;
  std::vector<std::vector<BooleanLiteral>> Encode(SolverState &state, std::vector<BooleanLiteral> lits) override;

  bool EncodingAddAtStart() override{ return false; };
  PropagatorEncoder() : IEncoder() {};

  ~PropagatorEncoder() override;
  class Factory : public IEncoder::IFactory {
    IEncoder *CallConstructor(std::vector<BooleanLiteral> literals, int min,
                              int max) override {
      return new PropagatorEncoder();
    };
  };
};
} // namespace Pumpkin
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_PROPAGATOR_ENCODER_H_
