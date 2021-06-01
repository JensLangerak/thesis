//
// Created by jens on 16-03-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANSUM_OUPUTPROPAGATOR_EXPLANATION_PB_SUM_CONSTRAINT_OUTPUT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANSUM_OUPUTPROPAGATOR_EXPLANATION_PB_SUM_CONSTRAINT_OUTPUT_H_

#include "../../../../Engine/solver_state.h"
#include "../../../explanation_generic.h"
#include "../watched_pb_sum_constraint.h"
namespace Pumpkin {
class ExplanationPbSumConstraintOutput : public ExplanationGeneric {
public:
  /// Get the explanation for the conflict
  /// \param constraint
  /// \param state
  ExplanationPbSumConstraintOutput(WatchedPbSumConstraint *constraint,
                             SolverState &state);
  /// Get the explanation for the propagated value.
  ExplanationPbSumConstraintOutput(WatchedPbSumConstraint *constraint,
                             SolverState &state,
                             BooleanLiteral propagated_literal);

  BooleanLiteral operator[](int index) const override;
  virtual BooleanLiteral operator[](size_t index) const;
  virtual size_t Size() const;
  ~ExplanationPbSumConstraintOutput(){};

private:
  std::vector<BooleanLiteral> lits_;
};
}

#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANSUM_INPUTPROPAGATOR_EXPLANATION_PB_SUM_CONSTRAINT_INPUT_H_
