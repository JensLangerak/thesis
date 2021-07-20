//
// Created by jens on 06-07-21.
//

#include "explanation_bottom_layers_constraint.h"
#include "../../../Engine/solver_state.h"

namespace Pumpkin {

void ExplanationBottomLayersConstraint::InitExplanationBottomLayers(
    WatchedBottomLayersConstraint *constraint, SolverState &state,
    ExplanationDynamicConstraint *explanation) {
  assert(constraint!= nullptr);
  assert(constraint->current_sum_value_ > constraint->max_);
  auto lits_ = std::vector<BooleanLiteral>();
  for (auto node : constraint->nodes_) {
    if (node.current_sum_ > 0) {
      for (int i = node.sum_literals.size() -1; i>=0; i--) {
        if (state.assignments_.IsAssignedTrue(node.sum_literals[i].literal)) {
          lits_.push_back(node.sum_literals[i].literal);
          break;
        }
      }
    }
  }
  explanation->Init(lits_);

}
void ExplanationBottomLayersConstraint::InitExplanationBottomLayers(
    WatchedBottomLayersConstraint *constraint, SolverState &state,
    BooleanLiteral propagated_literal,
    ExplanationDynamicConstraint *explanation) {
  assert(constraint!= nullptr);
  assert(state.assignments_.IsAssignedTrue(propagated_literal));
  auto lits_ = std::vector<BooleanLiteral>();
  for (auto node : constraint->nodes_) {
    if (node.current_sum_ > 0) {
      for (int i = node.sum_literals.size() -1; i>=0; i--) {
        if (state.assignments_.IsAssignedTrue(node.sum_literals[i].literal) && state.assignments_.GetTrailPosition(node.sum_literals[i].literal.Variable()) < state.assignments_.GetTrailPosition(propagated_literal.Variable())) {
          if (node.sum_literals[i].literal == (~propagated_literal))
            break;
          lits_.push_back(node.sum_literals[i].literal);
          break;
        }
      }
    }
  }
  explanation->Init(lits_);

}
}