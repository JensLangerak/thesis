//
// Created by jens on 18-06-21.
//

#include "propagator_pb_sum_input.h"
#include "../../../../Engine/solver_state.h"
#include "../../watched_dynamic_constraint.h"
#include "../watched_pb_sum_constraint.h"

namespace Pumpkin {

PropagatorPbSumInput::PropagatorPbSumInput(int64_t num_variables) : pb_sum_database_(num_variables){

}

bool PropagatorPbSumInput::PropagateLiteral(BooleanLiteral true_literal,
                                            SolverState &state) {
  assert(state.assignments_.IsAssignedTrue(true_literal));
  auto &watch_list_true = pb_sum_database_.watch_list_input_;
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

  for (; current_index < watchers_true.size(); ++current_index) {
    WatchedPbSumConstraint *constraint =
        watchers_true[current_index].constraint_;
    // if the encoding is added, no need to keep track of it anymore
    if (constraint->encoder_->EncodingAdded())
      continue;
    assert(constraint->true_count_ <= constraint->current_max_);
    bool count_updated = false;
    bool prev_already_done = already_partly_done;
    //    int last_ind = last_index_;
    if (last_propagation_info_.last_watcher_index != current_index ||
        (!already_partly_done)) {
      constraint->set_literals_.push_back(true_literal);
      int index = constraint->lit_to_input_index_[true_literal];
      int w = constraint->inputs_[index].weight;
      constraint->true_count_ +=w;

    }
    assert(constraint->CountCorrect(state, true_literal));

    already_partly_done = false;
    last_propagation_info_.last_watcher_index = current_index;
    int true_count = 0;
    true_count = constraint->true_count_;

    assert(watchers_true.size() > last_propagation_info_.end_index);
    assert(current_index <= last_propagation_info_.end_index);
    watchers_true[last_propagation_info_.end_index] =
        watchers_true[current_index];
    ++last_propagation_info_.end_index;
    bool success = constraint->current_max_ >= constraint->true_count_;
    if (success) {
      for (WeightedLiteral o : constraint->outputs_) {
        if (o.weight <= constraint->true_count_) {
          success = state.EnqueuePropagatedLiteral(
              o.literal, this, reinterpret_cast<uint64_t>(constraint));
          if (!success)
            break;
        }
      }
    }

    if (success) {
      success = PropagateInputs(constraint, state);
      assert(success);
    } else {

      failure_constraint_ = constraint;
      constraint->trigger_count_++;
      return false;
    }
  }

  next_position_on_trail_to_propagate_++;
  return true;
}
bool PropagatorPbSumInput::PropagateInputs(WatchedPbSumConstraint *constraint,
                                           SolverState &state) {
  int max_w = constraint->max_ - constraint->true_count_;
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

void PropagatorPbSumInput::Synchronise(SolverState &state) {
  PropagatorDynamic::Synchronise(state);
  for (auto constraint : pb_sum_database_.permanent_constraints_) {
    constraint->Synchronise(state);
    if (next_position_on_trail_to_propagate_ > 0) {
      assert(constraint->CountCorrect(state, state.GetLiteralFromTrailAtPosition(next_position_on_trail_to_propagate_ - 1)));
    }}
}
}