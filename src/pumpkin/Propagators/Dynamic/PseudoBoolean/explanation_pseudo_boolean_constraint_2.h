//
// Created by jens on 24-05-21.
//

#ifndef PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_EXPLANATION_PSEUDO_BOOLEAN_CONSTRAINT_2_H_
#define PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_EXPLANATION_PSEUDO_BOOLEAN_CONSTRAINT_2_H_

#include "../explanation_dynamic_constraint.h"
#include "watched_pseudo_boolean_constraint_2.h"
namespace Pumpkin {
class ExplanationPseudoBooleanConstraint2 {
public:
  /// Get the explanation for the conflict
  /// \param constraint
  /// \param state
  static void InitExplanationPseudoBooleanConstraint2(WatchedPseudoBooleanConstraint2 * constraint, SolverState &state, ExplanationDynamicConstraint * explanation);
  /// Get the explanation for the propagated value.
  static void InitExplanationPseudoBooleanConstraint2(WatchedPseudoBooleanConstraint2 * constraint, SolverState &state, BooleanLiteral propagated_literal, ExplanationDynamicConstraint * explanation);

};
} // namespace Pumpkin

#endif // PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_EXPLANATION_PSEUDO_BOOLEAN_CONSTRAINT_2_H_
