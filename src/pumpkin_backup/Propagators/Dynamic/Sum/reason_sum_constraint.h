//
// Created by jens on 17-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_SUM_REASON_SUM_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_SUM_REASON_SUM_CONSTRAINT_H_

#include "../../../Engine/solver_state.h"
#include "explanation_sum_constraint.h"
#include "watched_sum_constraint.h"
namespace Pumpkin {
class ReasonSumConstraint : public ReasonGeneric {
public:
  ReasonSumConstraint(WatchedSumConstraint *constraint,
                              SolverState &state);

  ReasonSumConstraint(WatchedSumConstraint *constraint,
                              BooleanLiteral propagated_literal,
                              SolverState &state);

  void RoundToOne(BooleanLiteral resolving_literal, SolverState &state) override;
  Term operator[](size_t index) override;
  uint64_t GetCoefficient(BooleanLiteral literal) override;
  void MultiplyByFraction(uint64_t numerator, uint64_t denominator) override;
  uint32_t GetRightHandSide() override;
  size_t Size() override;

private:
  ExplanationSumConstraint *explanation_;
  uint32_t scaling_factor_;
};
} // namespace Pumpkin
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_REASON_CARDINALITY_CONSTRAINT_H_
