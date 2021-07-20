//
// Created by jens on 06-07-21.
//

#include "propagator_bottom_layers.h"
#include "watched_bottom_layers_constraint.h"
#include "../../../Engine/solver_state.h"

namespace Pumpkin {
PropagatorBottomLayers::PropagatorBottomLayers(int64_t num_variables) : database_bottom_layers_(num_variables){}
void PropagatorBottomLayers::Synchronise(SolverState &state) {
  PropagatorDynamic::Synchronise(state);
  for (auto constraint : database_bottom_layers_.permanent_constraints_) {
    constraint->Synchronise(state);
//    if (next_position_on_trail_to_propagate_ > 0) {
//      assert(constraint->CountCorrect(state, state.GetLiteralFromTrailAtPosition(next_position_on_trail_to_propagate_ - 1)));
//    }
    assert(constraint->CountCorrect(state, state.GetLiteralFromTrailAtPosition(next_position_on_trail_to_propagate_ - 1)));
  }
}
bool PropagatorBottomLayers::PropagateLiteral(BooleanLiteral true_literal,
                                              SolverState &state) {
  assert(state.assignments_.IsAssignedTrue(true_literal));
  auto &watch_list_true = database_bottom_layers_.watch_list_true_;
  auto &watchers_true =watch_list_true[true_literal];
  size_t current_index = 0;
  // Check if it has partly propagated the literal or not
  bool already_partly_done = false;
  BooleanLiteral last = last_propagation_info_.last_propagated;
  if (!last_propagation_info_.last_propagated.IsUndefined() && last_propagation_info_.last_propagated == true_literal) {
    current_index = last_propagation_info_.last_watcher_index;
    already_partly_done = true;
  } else {
    last_propagation_info_.last_watcher_index = 0;
    last_propagation_info_.end_index = 0;
  }
  last_propagation_info_.last_propagated = true_literal;
//  assert(CheckCounts(state));
  // update all constraints that watch the literal
  for (; current_index < watchers_true.size(); ++current_index) {
    WatchedBottomLayersConstraint *constraint =
        watchers_true[current_index].constraint_;
    // if the encoding is added, no need to keep track of it anymore
    // only update the count the first time that the constraint is triggered.
    // //TODO not sure if the check is still needed
    bool count_updated = false;
    bool prev_already_done = already_partly_done;
//    int last_ind = last_index_;
    if (last_propagation_info_.last_watcher_index != current_index || (!already_partly_done)) {
      count_updated = true;
      constraint->UpdateSum(true_literal, watchers_true[current_index].weight_, state);
      constraint->UpdateDecisionLevelSum(state);
    }
   assert(constraint->CountCorrect(state, true_literal));

    already_partly_done = false;
    last_propagation_info_.last_watcher_index = current_index;
    int true_count = 0;
//    CheckCount(state, constraint);
    true_count = constraint->current_sum_value_;

    // conflict
    if (true_count > constraint->max_) {
      failure_constraint_ = constraint;
      constraint->trigger_count_++;
      return false;
    }

    int slack = constraint->max_ - true_count;

    for (int i = 0; i < constraint->nodes_.size(); ++i) {
      for (WeightedLiteral l : constraint->nodes_[i].sum_literals) {
        if (!state.assignments_.IsAssigned(l.literal)) {
          if (slack < l.weight) {
            state.EnqueuePropagatedLiteral(
                ~l.literal, this, reinterpret_cast<uint64_t>(constraint));
          }
        }
      }
    }
  }

  next_position_on_trail_to_propagate_++;
  return true;
}
}