//
// Created by jens on 18-06-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_SUM_OUTPUT_EXPLANATION_PB_SUM_CONSTRAINT_OUTPUT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_SUM_OUTPUT_EXPLANATION_PB_SUM_CONSTRAINT_OUTPUT_H_
#include "../../explanation_dynamic_constraint.h"
#include "../watched_pb_sum_constraint.h"
namespace Pumpkin {
class ExplanationPbSumConstraintOutput {
public:
  /// Get the explanation for the conflict
  /// \param constraint
  /// \param state
  static void InitExplanationPbSumOutput(WatchedPbSumConstraint * constraint, SolverState &state, ExplanationDynamicConstraint * explanation);
  /// Get the explanation for the propagated value.
  static void InitExplanationPbSumOutput(WatchedPbSumConstraint * constraint, SolverState &state, BooleanLiteral propagated_literal, ExplanationDynamicConstraint * explanation);

};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_SUM_OUTPUT_EXPLANATION_PB_SUM_CONSTRAINT_OUTPUT_H_
