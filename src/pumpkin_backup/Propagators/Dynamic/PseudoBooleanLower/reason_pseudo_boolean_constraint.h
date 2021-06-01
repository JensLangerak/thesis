//
// Created by jens on 10-12-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANLOWER_REASON_PSEUDO_BOOLEAN_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANLOWER_REASON_PSEUDO_BOOLEAN_CONSTRAINT_H_

#include "../../reason_generic.h"
#include "explanation_pseudo_boolean_constraint.h"
namespace Pumpkin {
class ReasonPseudoBooleanConstraint3 : public ReasonGeneric {
public:
  ReasonPseudoBooleanConstraint3(WatchedPseudoBooleanConstraint3 *constraint,
                              SolverState &state);

  ReasonPseudoBooleanConstraint3(WatchedPseudoBooleanConstraint3 *constraint,
                              BooleanLiteral propagated_literal,
                              SolverState &state);

  void RoundToOne(BooleanLiteral resolving_literal, SolverState &state) override;
  Term operator[](size_t index) override;
  uint64_t GetCoefficient(BooleanLiteral literal) override;
  void MultiplyByFraction(uint64_t numerator, uint64_t denominator) override;
  uint32_t GetRightHandSide() override;
  size_t Size() override;

private:
  ExplanationPseudoBooleanConstraint3 *explanation_;
  uint32_t scaling_factor_;
};
} // namespace Pumpkin
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_REASON_PSEUDO_BOOLEAN_CONSTRAINT_H_
