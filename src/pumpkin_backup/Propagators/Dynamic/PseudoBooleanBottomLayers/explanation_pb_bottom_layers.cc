//
// Created by jens on 18-05-21.
//


#include "explanation_pb_bottom_layers.h"
#include "../../../../logger/logger.h"
#include "watched_pb_bottom_layers_constraint.h"
#include "../../../Engine/solver_state.h"
#include "watched_pb_bottom_layers_constraint.h"

namespace Pumpkin {

BooleanLiteral ExplanationPbBottomLayers::operator[](int index) const {
  assert(index < lits_.size());
  return lits_[index];
}
BooleanLiteral ExplanationPbBottomLayers::operator[](size_t index) const {
  assert(index < lits_.size());
  return lits_[index];
}
ExplanationPbBottomLayers::ExplanationPbBottomLayers(
    WatchedPbBottomLayersConstraint *constraint, SolverState &state) {
  assert(constraint!= nullptr);
  assert(constraint->current_sum_value_ > constraint->max_);
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

}
ExplanationPbBottomLayers::ExplanationPbBottomLayers(
    WatchedPbBottomLayersConstraint *constraint, SolverState &state,
    BooleanLiteral propagated_literal) {
  assert(constraint!= nullptr);
  assert(state.assignments_.IsAssignedTrue(propagated_literal));
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
  lits_.push_back(~propagated_literal); //TODO not sure if correct?
}
size_t ExplanationPbBottomLayers::Size() const { return lits_.size(); }
} // namespace Pumpkin