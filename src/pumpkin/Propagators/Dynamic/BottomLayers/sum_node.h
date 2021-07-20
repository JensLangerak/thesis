//
// Created by jens on 07-07-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_BOTTOMLAYERS_SUM_NODE_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_BOTTOMLAYERS_SUM_NODE_H_

#include "../../../Utilities/problem_specification.h"
#include <vector>
namespace Pumpkin {
class SumNode {
public:
  SumNode(WeightedLiteral literal, WeightedLiteral literal_1,
          SolverState &state);
  explicit SumNode(std::vector<WeightedLiteral>& sum_lits, std::vector<WeightedLiteral> &inputs) : sum_literals(), current_sum_(0) {
    for (auto w : sum_lits)
      this->sum_literals.push_back(w);
    for (auto w : inputs)
      this->inputs_debug.push_back(w);
  };
  std::vector<WeightedLiteral> sum_literals;
  std::vector<WeightedLiteral> inputs_debug;
  uint32_t current_sum_ = 0;
};
}

#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_BOTTOMLAYERS_SUM_NODE_H_
