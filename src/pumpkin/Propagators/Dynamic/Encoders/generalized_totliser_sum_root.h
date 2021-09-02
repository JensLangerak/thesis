//
// Created by jens on 21-03-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_ENCODERS_GENERALIZED_TOTLISER_SUM_ROOT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_ENCODERS_GENERALIZED_TOTLISER_SUM_ROOT_H_

#include "../../../Utilities//boolean_literal.h"
#include "../../../Utilities//problem_specification.h"
#include "generalized_totaliser_sum_nodes.h"
#include "i_encoder.h"
#include <cassert>
#include <unordered_map>
#include <vector>

namespace Pumpkin {
class GeneralizedTotliserSumRoot  : public IEncoder<PseudoBooleanConstraint> {
public:
  GeneralizedTotliserSumRoot(std::vector<BooleanLiteral> variables, std::vector<uint32_t > weights, int max);

  std::vector<WeightedLiteral> inputs_;
  int max_;

  void Encode(SolverState &state) override;
  void Encode(SolverState &state, std::vector<BooleanLiteral> lits) override;

  GeneralizedTotaliserSumNodes *root_node_;

  bool UpdateMax(int max, SolverState &state) override;

  class Factory : public IEncoder<PseudoBooleanConstraint>::IFactory {
  public:
    Factory(EncodingStrategy strategy, double delay_factor) : IEncoder<PseudoBooleanConstraint>::IFactory(strategy, delay_factor) {};
    ~Factory() override = default;

  protected:
    IEncoder<PseudoBooleanConstraint> * CallConstructor(PseudoBooleanConstraint &constraint) override {return new GeneralizedTotliserSumRoot(constraint.literals, constraint.coefficients, constraint.right_hand_side);};
  };
  WatchedPbSumConstraint *root;
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_ENCODERS_GENERALIZED_TOTLISER_SUM_ROOT_H_
