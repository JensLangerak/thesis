//
// Created by jens on 10-12-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANLOWER_EXPLANATION_PSEUDO_BOOLEAN_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANLOWER_EXPLANATION_PSEUDO_BOOLEAN_CONSTRAINT_H_

#include "../../explanation_generic.h"
#include "watched_pseudo_boolean_constraint.h"
namespace Pumpkin {
class ExplanationPseudoBooleanConstraint3 : public ExplanationGeneric {
public:
  /// Get the explanation for the conflict
  /// \param constraint
  /// \param state
  ExplanationPseudoBooleanConstraint3(WatchedPseudoBooleanConstraint3 * constraint, SolverState &state);
  /// Get the explanation for the propagated value.
  ExplanationPseudoBooleanConstraint3(WatchedPseudoBooleanConstraint3 * constraint, SolverState &state, BooleanLiteral propagated_literal);

  BooleanLiteral operator[](int index) const override;
  virtual BooleanLiteral operator[](size_t index) const;
  virtual size_t Size() const;
  ~ExplanationPseudoBooleanConstraint3() {};

private:
  std::vector<BooleanLiteral> lits_;
  void ReplaceLits(std::vector<WeightedLiteral> causes, SolverState &state);
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_EXPLANATION_PSEUDO_BOOLEAN_CONSTRAINT_H_
