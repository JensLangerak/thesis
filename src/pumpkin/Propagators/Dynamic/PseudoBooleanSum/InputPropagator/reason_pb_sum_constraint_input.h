//
// Created by jens on 16-03-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANSUM_INPUTPROPAGATOR_REASON_PB_SUM_CONSTRAINT_INPUT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANSUM_INPUTPROPAGATOR_REASON_PB_SUM_CONSTRAINT_INPUT_H_

#include "../../../../Engine/solver_state.h"
#include "../watched_pb_sum_constraint.h"
#include "explanation_pb_sum_constraint_input.h"
namespace Pumpkin {
class ReasonPbSumConstraintInput : public ReasonGeneric {
public:
  ReasonPbSumConstraintInput(WatchedPbSumConstraint *constraint,
  SolverState &state);

  ReasonPbSumConstraintInput(WatchedPbSumConstraint *constraint,
  BooleanLiteral propagated_literal,
      SolverState &state);

  void RoundToOne(BooleanLiteral resolving_literal, SolverState &state) override;
  Term operator[](size_t index) override;
  uint64_t GetCoefficient(BooleanLiteral literal) override;
  void MultiplyByFraction(uint64_t numerator, uint64_t denominator) override;
  uint32_t GetRightHandSide() override;
  size_t Size() override;

private:
  ExplanationPbSumConstraintInput *explanation_;
  uint32_t scaling_factor_;
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANSUM_INPUTPROPAGATOR_REASON_PB_SUM_CONSTRAINT_INPUT_H_
