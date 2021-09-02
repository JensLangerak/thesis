//
// Created by jens on 17-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_EXPLANATION_CARDINALITY_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_EXPLANATION_CARDINALITY_CONSTRAINT_H_

#include "../../../Engine/solver_state.h"
#include "../../explanation_generic.h"
#include "watched_cardinality_constraint.h"
namespace Pumpkin {
class ExplanationCardinalityConstraint : public ExplanationGeneric {
public:
  /// Get the explanation for the conflict
  /// \param constraint
  /// \param state
  ExplanationCardinalityConstraint(WatchedCardinalityConstraint * constraint, SolverState &state);
  /// Get the explanation for the propagated value.
  ExplanationCardinalityConstraint(WatchedCardinalityConstraint * constraint, SolverState &state, BooleanLiteral propagated_literal);

  BooleanLiteral operator[](int index) const override;
  virtual BooleanLiteral operator[](size_t index) const;
  virtual size_t Size() const;
  ~ExplanationCardinalityConstraint() {};

private:
  std::vector<BooleanLiteral> lits_;
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_SUM_EXPLANATION_SUM_CONSTRAINT_H_
