//
// Created by jens on 22-03-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_ENCODERS_GENERALIZED_TOTALISER_SUM_HELPERS_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_ENCODERS_GENERALIZED_TOTALISER_SUM_HELPERS_H_

#include "../../../Utilities/boolean_literal.h"
#include "../../../Utilities/problem_specification.h"
#include "i_encoder.h"
#include <cassert>
#include <unordered_map>
#include <vector>

namespace Pumpkin {
class WatchedPbSumConstraint;
class GeneralizedTotaliserSumNodes;
class GeneralizedTotaliserSumHelpers {
public:
  static std::vector<WeightedLiteral>
  CreateSumLiterals(const std::vector<WeightedLiteral>& inputs, int max, SolverState &state);
  static WatchedPbSumConstraint* AddPbSumConstraint(std::vector<WeightedLiteral> inputs,
                          std::vector<WeightedLiteral> outputs,
                          int max,
                          double add_delay,
                          EncodingStrategy encoding_strategy,
                          SolverState &state,
                          GeneralizedTotaliserSumNodes *& encoder_result);
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_ENCODERS_GENERALIZED_TOTALISER_SUM_HELPERS_H_
