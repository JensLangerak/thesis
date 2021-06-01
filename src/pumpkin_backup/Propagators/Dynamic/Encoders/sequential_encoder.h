//
// Created by jens on 28-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_SEQUENTIAL_ENCODER_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_SEQUENTIAL_ENCODER_H_

#include "../../../Basic Data Structures/boolean_literal.h"
#include "i_encoder.h"
#include <vector>
#include "../../../Basic Data Structures/problem_specification.h"
namespace Pumpkin {
class SequentialEncoder : public IEncoder<CardinalityConstraint> {

public:
  std::vector<std::vector<BooleanLiteral>>
  Encode(SolverState &state) override;

  SequentialEncoder(std::vector<BooleanLiteral> variables, int min, int max);

  ~SequentialEncoder() override;

  class Factory : public IEncoder<CardinalityConstraint>::IFactory {
    IEncoder<CardinalityConstraint> * CallConstructor(CardinalityConstraint &constraint) override;
  };
private:
  std::vector<BooleanLiteral> variables_;
  std::vector<std::vector<BooleanLiteral>> added_clauses_;
  int min_;
  int max_;
};
}

#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_SEQUENTIAL_ENCODER_H_
