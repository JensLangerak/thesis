//
// Created by jens on 18-06-21.
//

#include "propagator_pb_sum_output.h"
#include "../../../../Engine/solver_state.h"

namespace Pumpkin {

bool PropagatorPbSumOutput::PropagateLiteral(BooleanLiteral true_literal,
                                             SolverState &state) {
  assert(state.assignments_.IsAssignedTrue(true_literal));
  BooleanLiteral false_lit = ~true_literal;
  auto &watch_list_output = pb_sum_database_->watch_list_output_;
  auto &watchers_false =
      watch_list_output[true_literal];
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

  // update all constraints that watch the literal
  for (; current_index < watchers_false.size(); ++current_index) {
    WatchedPbSumConstraint *constraint =
        watchers_false[current_index].constraint_;
    // if the encoding is added, no need to keep track of it anymore
    if (constraint->encoder_->EncodingAdded())
      continue;
    assert(constraint->true_count_ <= constraint->current_max_);
    // only update the count the first time that the constraint is triggered.
    // //TODO not sure if the check is still needed

    if (last_propagation_info_.last_watcher_index != current_index ||
        (!already_partly_done)) {
      int index = constraint->lit_to_output_index_[false_lit];
      WeightedLiteral wl = constraint->outputs_[index];
      if (wl.weight > constraint->max_) {
        already_partly_done = false;
        last_propagation_info_.last_watcher_index = current_index;
        continue;
      }
      if (wl.weight < constraint->current_max_) {
        constraint->set_outputs_.push_back(wl);
        constraint->current_max_ = wl.weight - 1;
      } else {
        already_partly_done = false;
        last_propagation_info_.last_watcher_index = current_index;
        continue;
      }
    }

    //    assert(constraint->true_count_ >= constraint->weight_trail_.back());
    //TODO move up
    already_partly_done = false;
    last_propagation_info_.last_watcher_index = current_index;

    bool success = constraint->current_max_ >= constraint->true_count_;
    if (success) {
      success = PropagateInputs(constraint, state);
    }

    //    success = EnqueTrailWeights(constraint, state);

    if (!success) {
      failure_constraint_ = constraint;
      //      return false;
      constraint->trigger_count_++;
      return false;
    }
  }

  next_position_on_trail_to_propagate_++;
  return true;


}

bool PropagatorPbSumOutput::PropagateInputs(WatchedPbSumConstraint *constraint,
                                            SolverState &state) {
  int max_w = constraint->current_max_ - constraint->true_count_;
  if (max_w >= constraint->max_weight_)
    return true;

  bool success = true;

  for (WeightedLiteral il : constraint->inputs_) {
    if (!state.assignments_.IsAssigned(il.literal)) {
      if (max_w < il.weight) {
        success = state.EnqueuePropagatedLiteral(
            ~il.literal, this, reinterpret_cast<uint64_t>(constraint));
        if (!success)
          return false;
      }
    }
  }

  return success;
}
void PropagatorPbSumOutput::Synchronise(SolverState &state) {
  PropagatorDynamic::Synchronise(state);
}
}