//
// Created by jens on 28-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_I_ENCODER_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_I_ENCODER_H_

#include <vector>
namespace Pumpkin {
class SolverState;
class BooleanLiteral;
class IEncoder {
public:
  virtual std::vector<std::vector<BooleanLiteral>> Encode(SolverState &state) = 0;
  virtual ~IEncoder() = 0;

protected:
  IEncoder(){};
};
}

#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_I_ENCODER_H_
