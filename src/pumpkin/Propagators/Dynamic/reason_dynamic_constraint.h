//
// Created by jens on 20-05-21.
//

#ifndef PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_REASON_DYNAMIC_CONSTRAINT_H_
#define PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_REASON_DYNAMIC_CONSTRAINT_H_

#include "../reason_generic.h"
#include "explanation_dynamic_constraint.h"
namespace Pumpkin {
class ReasonDynamicConstraint : public ReasonGeneric {
public:
  explicit ReasonDynamicConstraint(ExplanationGeneric *explanation);


  void RoundToOne(BooleanLiteral resolving_literal, SolverState &state) override;
  Term operator[](size_t index) override;
  uint64_t GetCoefficient(BooleanLiteral literal) override;
  void MultiplyByFraction(uint64_t numerator, uint64_t denominator) override;
  uint32_t GetRightHandSide() override;
  size_t Size() override;

  ~ReasonDynamicConstraint() override;
protected:
  ExplanationGeneric *explanation_;
  uint32_t scaling_factor_;
};

} // namespace Pumpkin

#endif // PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_REASON_DYNAMIC_CONSTRAINT_H_
