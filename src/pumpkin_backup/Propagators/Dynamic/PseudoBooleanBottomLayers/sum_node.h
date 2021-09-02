//
// Created by jens on 25-05-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANBOTTOMLAYERS_SUM_NODE_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANBOTTOMLAYERS_SUM_NODE_H_

#include "../../../Basic Data Structures/problem_specification.h"
#include <vector>
namespace Pumpkin {
class SumNode {
public:
  SumNode(WeightedLiteral literal, WeightedLiteral literal_1,
          SolverState &state);
  explicit SumNode(std::vector<WeightedLiteral>& sum_lits) : sum_literals(), current_sum_(0) {
                                                                                    for (auto w : sum_lits)
                                                                                      this->sum_literals.push_back(w);
                                                                                };
  std::vector<WeightedLiteral> sum_literals;
  uint32_t current_sum_ = 0;
};
}

#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANBOTTOMLAYERS_SUM_NODE_H_
