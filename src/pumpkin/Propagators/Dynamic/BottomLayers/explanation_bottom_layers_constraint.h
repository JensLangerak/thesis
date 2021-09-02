//
// Created by jens on 06-07-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_BOTTOMLAYERS_EXPLANATION_BOTTOM_LAYERS_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_BOTTOMLAYERS_EXPLANATION_BOTTOM_LAYERS_CONSTRAINT_H_
#include "../explanation_dynamic_constraint.h"
#include "watched_bottom_layers_constraint.h"
namespace Pumpkin {
class SolverState;
class ExplanationBottomLayersConstraint {
public:
  /// Get the explanation for the conflict
  /// \param constraint
  /// \param state
  static void InitExplanationBottomLayers(WatchedBottomLayersConstraint * constraint, SolverState &state, ExplanationDynamicConstraint * explanation);
  /// Get the explanation for the propagated value.
  static void InitExplanationBottomLayers(WatchedBottomLayersConstraint * constraint, SolverState &state, BooleanLiteral propagated_literal, ExplanationDynamicConstraint * explanation);

};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_BOTTOMLAYERS_EXPLANATION_BOTTOM_LAYERS_CONSTRAINT_H_
