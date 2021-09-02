//
// Created by jens on 16-10-20.
//

#include "propagator_sum.h"
#include "../../../Engine/solver_state.h"
#include "../Encoders/incremental_sequential_encoder.h"
#include "../Encoders/totaliser_encoder.h"
#include "reason_sum_constraint.h"
#include "watch_list_sum.h"
#include <iostream>
namespace Pumpkin {

PropagatorSum::PropagatorSum(int64_t num_variables)
    : PropagatorDynamic(), sum_database_(num_variables), last_index_(0),
      last_propagated_(BooleanLiteral()) {}
bool PropagatorSum::PropagateLiteral(BooleanLiteral true_literal,
                                             SolverState &state) {
  assert(state.assignments_.IsAssignedTrue(true_literal));
  WatchListSum &watch_list_true = sum_database_.watch_list_true;
  std::vector<WatcherSumConstraint> &watchers_true =
      watch_list_true[true_literal];
  size_t end_position = 0;
  size_t current_index = 0;
  // Check if it has partly propagated the literal or not
  bool already_partly_done = false;
  if (!last_propagated_.IsUndefined() && last_propagated_ == true_literal) {
    current_index = last_index_;
    already_partly_done = true;
  } else {
    last_index_ = 0;
  }
  last_propagated_ = true_literal;
  assert(CheckCounts(state));
  // update all constraints that watch the literal
  for (; current_index < watchers_true.size(); ++current_index) {
    WatchedSumConstraint *constraint =
        watchers_true[current_index].constraint_;
    // if the encoding is added, no need to keep track of it anymore
    if (constraint->encoder_->EncodingAdded())
      continue;
    // only update the count the first time that the constraint is triggered.
    // //TODO not sure if the check is still needed
    if (last_index_ != current_index || (!already_partly_done)) {
      constraint->true_count_++;
      constraint->true_order_.push_back(true_literal);
    }
    already_partly_done = false;
    last_index_ = current_index;
    int true_count = 0;
    int false_count = 0;
    for (BooleanLiteral l : constraint->inputs_) {
      if (state.assignments_.IsAssignedTrue(l) &&
          state.assignments_.GetTrailPosition(l.Variable()) <=
              state.assignments_.GetTrailPosition(true_literal.Variable()))
        true_count++;
    }
//        assert(true_count == constraint->true_count_);
    constraint->true_count_ = true_count;
    true_count = constraint->true_count_;

    watchers_true[end_position] = watchers_true[current_index];
    ++end_position;
    if (true_count > constraint->outputs_.size())
      continue;
    bool success = state.EnqueuePropagatedLiteral(constraint->outputs_[true_count - 1], this, reinterpret_cast<uint64_t>(constraint));

    // conflict
    if (!success) {
      // restore remaining watchers
      for (size_t k = current_index + 1; k < watchers_true.size(); ++k) {
        watchers_true[end_position] = watchers_true[current_index];
        ++end_position;
      }
      watchers_true.resize(end_position);
      failure_constraint_ = constraint;
      //      return false;
      constraint->trigger_count_++;
      trigger_count_++;
      if (constraint->encoder_->AddEncodingDynamic()) {
        bool res = AddEncoding(state, constraint);
        //        state.FullReset();
        return res;
      } else {
        auto test1 = state.assignments_.IsAssigned(constraint->outputs_[true_count - 1]);
        auto test2 = state.assignments_.IsAssignedTrue(constraint->outputs_[true_count - 1]);
        return false;
      }
    }

  }

  watchers_true.resize(end_position);
  if (!next_position_on_trail_to_propagate_it.IsPastTrail())
    next_position_on_trail_to_propagate_it.Next();
  //  assert(CheckCounts(state));
  return true;
}
ReasonGeneric *PropagatorSum::ReasonFailure(SolverState &state) {
  return new ReasonSumConstraint(failure_constraint_, state);
}
ReasonGeneric *PropagatorSum::ReasonLiteralPropagation(
    BooleanLiteral propagated_literal, SolverState &state) {
  assert(state.assignments_.GetAssignmentPropagator(
             propagated_literal.Variable()) == this);
  uint64_t code =
      state.assignments_.GetAssignmentCode(propagated_literal.Variable());
  WatchedSumConstraint *propagating_constraint =
      reinterpret_cast<WatchedSumConstraint *>(code);
  // TODO check if encoding should be added (if called it is not yet added)
  return new ReasonSumConstraint(propagating_constraint,
                                         propagated_literal, state);
}
ExplanationGeneric *PropagatorSum::ExplainFailure(SolverState &state) {
  return failure_constraint_->ExplainFailure(state);
}
ExplanationGeneric *PropagatorSum::ExplainLiteralPropagation(
    BooleanLiteral propagated_literal, SolverState &state) {
  assert(state.assignments_.GetAssignmentPropagator(
             propagated_literal.Variable()) == this);
  uint64_t code =
      state.assignments_.GetAssignmentCode(propagated_literal.Variable());
  WatchedSumConstraint *propagating_constraint =
      reinterpret_cast<WatchedSumConstraint *>(code);
  // TODO check if encoding should be added (if called it is not yet added)
  return propagating_constraint->ExplainLiteralPropagation(propagated_literal,
                                                           state);
}

void PropagatorSum::Synchronise(SolverState &state) {

  // current literal is partly propagated, reduce the counts of the updated
  // constraints.
  if ((!last_propagated_.IsUndefined()) &&
      last_propagated_ == next_position_on_trail_to_propagate_it.GetData()) {
    BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
    if (!sum_database_.watch_list_true[l].empty()) {
      assert(last_index_ < sum_database_.watch_list_true[l].size());
      for (int i = 0; i <= last_index_; ++i) {
        sum_database_.watch_list_true[l][i].constraint_->true_count_--;

        sum_database_.watch_list_true[l][i].constraint_->true_order_.pop_back();
      }
      last_index_ = 0;
      // TODO roll back false
    }
  }

  // move back the iterator and updates the counts
  if (next_position_on_trail_to_propagate_it.IsPastTrail()) {
    while (next_position_on_trail_to_propagate_it != state.GetTrailEnd()) {
      next_position_on_trail_to_propagate_it.Previous();
      BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
      for (auto wc : sum_database_.watch_list_true[l]) {
        wc.constraint_->true_count_--;
        wc.constraint_->true_order_.pop_back();
      }
    }
    assert(next_position_on_trail_to_propagate_it == state.GetTrailEnd());
  }
  PropagatorGeneric::Synchronise(state);
  last_propagated_ = BooleanLiteral();
  last_index_ = 0;
}

bool PropagatorSum::PropagateOneLiteral(SolverState &state) {
  if (IsPropagationComplete(state) == false) {

    BooleanLiteral propagation_literal =
        *next_position_on_trail_to_propagate_it;
    bool success = PropagateLiteral(propagation_literal, state);
    return success;
  }
  return true; // no conflicts occurred during propagation
}

void PropagatorSum::ResetCounts() {
  for (auto c : sum_database_.permanent_constraints_) {
    c->true_count_ = 0;
    c->true_order_.clear();
    c->false_count_ = 0;
  }
}

void PropagatorSum::SetTrailIterator(
    TrailList<BooleanLiteral>::Iterator iterator) {
  // set the trail iterator and make sure the the counts stay correct.
  if (iterator.IsFirst()) {
    PropagatorGeneric::SetTrailIterator(iterator);
    ResetCounts();
  } else {

    if (!next_position_on_trail_to_propagate_it.IsPastTrail()) {
      BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
      for (auto wc : sum_database_.watch_list_true[l]) {
        wc.constraint_->true_count_--;
        wc.constraint_->true_order_.pop_back();
      }
    }
    while (next_position_on_trail_to_propagate_it != iterator) {
      next_position_on_trail_to_propagate_it.Previous();
      BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
      for (auto wc : sum_database_.watch_list_true[l]) {
        wc.constraint_->true_count_--;
        wc.constraint_->true_order_.pop_back();
      }
    }
  }
  assert(next_position_on_trail_to_propagate_it == iterator);
  last_propagated_ = BooleanLiteral();
  last_index_ = 0;
}
bool PropagatorSum::CheckCounts(SolverState &state) {
  return true;
  TrailList<BooleanLiteral>::Iterator counter_it = state.GetTrailBegin();
  for (auto c : sum_database_.permanent_constraints_) {
    c->true_count_debug_ = 0;
    //    c->true_log_debug.clear();
  }
  while (counter_it != next_position_on_trail_to_propagate_it) {
    BooleanLiteral l = counter_it.GetData();
    for (auto c : sum_database_.watch_list_true[l]) {
      c.constraint_->true_count_debug_++;
      //      c.constraint_->true_log_debug.emplace_back(c.constraint_->true_count_debug_,
      //      l);
    }
    counter_it.Next();
  }
  int miscount = 0;
  int l = 0;
  int g = 0;
  for (auto c : sum_database_.permanent_constraints_) {
    //    assert(c->true_count_debug_ == c->true_count_);
    if (c->true_count_debug_ != c->true_count_) {
      miscount++;
      if (c->true_count_debug_ > c->true_count_)
        ++g;
      else
        ++l;
    }
  }

  assert(miscount == 0);
  return true;
}


void PropagatorSum::PropagateIncremental2(SolverState &state, WatchedSumConstraint * constraint, std::vector<BooleanLiteral> &reason, std::vector<BooleanLiteral> &propagate) {
  assert(constraint != NULL);

  for (int i = 0; i < constraint->inputs_.size(); ++i) {
    BooleanLiteral l = constraint->inputs_[i];
    if ((!state.assignments_.IsAssigned(
            l.Variable()))) // && (!constraint->encoder_->IsAdded(l)))
      propagate.push_back(l);
    else if (state.assignments_.IsAssignedTrue(l))
      reason.push_back(l);
  }
}

std::vector<BooleanLiteral>
PropagatorSum::GetEncodingCause(SolverState &state, WatchedSumConstraint *constraint) {
    std::vector<BooleanLiteral> cause;
    int level_count = 0;
    for (BooleanLiteral l : constraint->inputs_) {
      if (state.assignments_.IsAssignedTrue(l)) {
        cause.push_back(l);
        if (state.assignments_.GetAssignmentLevel(l.Variable()) ==
            state.GetCurrentDecisionLevel())
          level_count++;
      }
    }
  return cause;
}

} // namespace Pumpkin
