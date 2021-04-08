//
// Created by jens on 16-03-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANPbSum_REASON_PB_PbSum_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANPbSum_REASON_PB_PbSum_CONSTRAINT_H_

#include "../../../Engine/solver_state.h"
#include "explanation_pb_sum_constraint.h"
#include "watched_pb_sum_constraint.h"
namespace Pumpkin {
class ReasonPbSumConstraint : public ReasonGeneric {
public:
  ReasonPbSumConstraint(WatchedPbSumConstraint *constraint,
  SolverState &state);

  ReasonPbSumConstraint(WatchedPbSumConstraint *constraint,
  BooleanLiteral propagated_literal,
      SolverState &state);

  void RoundToOne(BooleanLiteral resolving_literal, SolverState &state) override;
  Term operator[](size_t index) override;
  uint64_t GetCoefficient(BooleanLiteral literal) override;
  void MultiplyByFraction(uint64_t numerator, uint64_t denominator) override;
  uint32_t GetRightHandSide() override;
  size_t Size() override;

private:
  ExplanationPbSumConstraint *explanation_;
  uint32_t scaling_factor_;
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANPbSum_REASON_PB_PbSum_CONSTRAINT_H_
