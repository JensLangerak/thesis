//
// Created by jens on 22-03-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_ENCODERS_GENERALIZED_TOTALISER_SUM_HELPERS_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_ENCODERS_GENERALIZED_TOTALISER_SUM_HELPERS_H_

#include "../../../Basic Data Structures/boolean_literal.h"
#include "../../../Basic Data Structures/problem_specification.h"
#include "i_encoder.h"
#include <cassert>
#include <unordered_map>
#include <vector>
#include "../PseudoBooleanSum/watched_pb_sum_constraint.h"
#include "generalized_totaliser_sum_nodes.h"

namespace Pumpkin {
class GeneralizedTotaliserSumHelpers {
public:
  static std::vector<WeightedLiteral>
  CreateSumLiterals(const std::vector<WeightedLiteral>& inputs, int max, SolverState &state);
  static WatchedPbSumConstraint* AddPbSumConstraint(std::vector<WeightedLiteral> inputs,
                          std::vector<WeightedLiteral> outputs,
                          int max,
                          double add_delay,
                          bool add_incremental,
                          bool add_dynamic,
                          SolverState &state,
                          GeneralizedTotaliserSumNodes *& encoder_result);
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_ENCODERS_GENERALIZED_TOTALISER_SUM_HELPERS_H_
