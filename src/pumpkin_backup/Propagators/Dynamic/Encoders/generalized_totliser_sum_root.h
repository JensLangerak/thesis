//
// Created by jens on 21-03-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_ENCODERS_GENERALIZED_TOTLISER_SUM_ROOT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_ENCODERS_GENERALIZED_TOTLISER_SUM_ROOT_H_

#include "../../../Basic Data Structures/boolean_literal.h"
#include "../../../Basic Data Structures/problem_specification.h"
#include "generalized_totaliser_sum_helpers.h"
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

  std::vector<std::vector<BooleanLiteral>>
  Encode(SolverState &state) override;
  std::vector<std::vector<BooleanLiteral>>
  Encode(SolverState &state, std::vector<BooleanLiteral> lits) override;

  GeneralizedTotaliserSumNodes *root_node_;

  bool UpdateMax(int max, SolverState &state) override;
  bool AddOnRestart() override{ return true;};
  bool AddEncodingDynamic() override { return false;};

  class Factory : public IEncoder<PseudoBooleanConstraint>::IFactory {
    ~Factory() override = default;
    IEncoder<PseudoBooleanConstraint> * CallConstructor(PseudoBooleanConstraint &constraint) override {return new GeneralizedTotliserSumRoot(constraint.literals, constraint.coefficients, constraint.right_hand_side);};
  };
  WatchedPbSumConstraint *root;
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_ENCODERS_GENERALIZED_TOTLISER_SUM_ROOT_H_
