//
// Created by jens on 24-05-21.
//

#include "propagator_pseudo_boolean_2.h"
#include "../../../Engine/solver_state.h"
#include "../../../Utilities/problem_specification.h"
namespace Pumpkin {
bool PropagatorPseudoBoolean2::PropagateLiteral(BooleanLiteral true_literal,
                                                SolverState &state) {
  assert(state.assignments_.IsAssignedTrue(true_literal));
  auto &watch_list_true = pseudo_boolean_database_.watch_list_true_;
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
    WatchedPseudoBooleanConstraint2 *constraint =
        watchers_true[current_index].constraint_;
    // if the encoding is added, no need to keep track of it anymore
    if (constraint->encoder_->EncodingAdded())
      continue;
    // only update the count the first time that the constraint is triggered.
    // //TODO not sure if the check is still needed
    bool count_updated = false;
    bool prev_already_done = already_partly_done;
//    int last_ind = last_index_;
    if (last_propagation_info_.last_watcher_index != current_index || (!already_partly_done)) {
      count_updated = true;
      constraint->current_sum_value_+=watchers_true[current_index].weight_;
      constraint->UpdateDecisionLevelSum(state);
      assert(constraint->CountCorrect(state, true_literal));
    }
    assert(constraint->CountCorrect(state, true_literal));

    already_partly_done = false;
    last_propagation_info_.last_watcher_index = current_index;
    int true_count = 0;
    true_count = constraint->current_sum_value_;

    assert(watchers_true.size() > last_propagation_info_.end_index);
    assert(current_index <= last_propagation_info_.end_index);
    watchers_true[last_propagation_info_.end_index] = watchers_true[current_index];
    ++last_propagation_info_.end_index;
    // conflict
    if (true_count > constraint->max_) {
      // restore remaining watchers
      for (size_t k = current_index + 1; k < watchers_true.size(); ++k) {
        watchers_true[last_propagation_info_.end_index] = watchers_true[k];
        ++last_propagation_info_.end_index;
      }
      watchers_true.resize(last_propagation_info_.end_index);
      failure_constraint_ = constraint;
      return false;
    }

    int slack = constraint->max_ - true_count;

    for (int i = 0; i <constraint->current_literals_.size(); ++i) {
      WeightedLiteral l = constraint->current_literals_[i];
      if (slack >= l.weight)
        break;
        if (!state.assignments_.IsAssigned(l.literal)) {
            state.EnqueuePropagatedLiteral(
                ~l.literal, this, reinterpret_cast<uint64_t>(constraint));
          }
      }
    }

  watchers_true.resize(last_propagation_info_.end_index);
  next_position_on_trail_to_propagate_++;
  return true;
}
void PropagatorPseudoBoolean2::Synchronise(SolverState &state) {
  PropagatorDynamic::Synchronise(state);
  for (auto constraint : pseudo_boolean_database_.permanent_constraints_) {
    constraint->Synchronise(state);
    if (next_position_on_trail_to_propagate_ > 0)
    assert(constraint->CountCorrect(state, state.GetLiteralFromTrailAtPosition(next_position_on_trail_to_propagate_ - 1)));
  }


}
PropagatorPseudoBoolean2::PropagatorPseudoBoolean2(int64_t num_variables)    : PropagatorDynamic(), pseudo_boolean_database_(num_variables){}
}
