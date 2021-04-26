//
// Created by jens on 16-03-21.
//

#include "propagator_pb_sum_output.h"
#include "../../../../Engine/solver_state.h"
#include "../../Encoders/generalized_totaliser_sum_nodes.h"
#include "../../Encoders/incremental_sequential_encoder.h"
#include "../../Encoders/totaliser_encoder.h"
#include "../watch_list_pb_sum.h"
#include "reason_pb_sum_constraint_output.h"
#include <iostream>
#include <stack>
#include <unordered_set>
namespace Pumpkin {

PropagatorPbSumOutput::PropagatorPbSumOutput(int64_t num_variables)
    : PropagatorDynamic(), last_index_(0),
      last_propagated_(BooleanLiteral()) {}

ReasonGeneric *PropagatorPbSumOutput::ReasonFailure(SolverState &state) {
  return new ReasonPbSumConstraintOutput(failure_constraint_, state);
}
ReasonGeneric *PropagatorPbSumOutput::ReasonLiteralPropagation(
    BooleanLiteral propagated_literal, SolverState &state) {
  assert(state.assignments_.GetAssignmentPropagator(
             propagated_literal.Variable()) == this);
  uint64_t code =
      state.assignments_.GetAssignmentCode(propagated_literal.Variable());
  WatchedPbSumConstraint *propagating_constraint =
      reinterpret_cast<WatchedPbSumConstraint *>(code);
  // TODO check if encoding should be added (if called it is not yet added)
  return new ReasonPbSumConstraintOutput(propagating_constraint,
                                         propagated_literal, state);
}
ExplanationGeneric *PropagatorPbSumOutput::ExplainFailure(SolverState &state) {
  return failure_constraint_->ExplainFailureOutput(state);
}
ExplanationGeneric *PropagatorPbSumOutput::ExplainLiteralPropagation(
    BooleanLiteral propagated_literal, SolverState &state) {
  assert(state.assignments_.GetAssignmentPropagator(
             propagated_literal.Variable()) == this);
  uint64_t code =
      state.assignments_.GetAssignmentCode(propagated_literal.Variable());
  WatchedPbSumConstraint *propagating_constraint =
      reinterpret_cast<WatchedPbSumConstraint *>(code);
  // TODO check if encoding should be added (if called it is not yet added)
  return propagating_constraint->ExplainLiteralPropagationOutput(
      propagated_literal, state);
}

bool PropagatorPbSumOutput::PropagateOneLiteral(SolverState &state) {
  if (IsPropagationComplete(state) == false) {

    BooleanLiteral propagation_literal =
        *next_position_on_trail_to_propagate_it;
    bool success = PropagateLiteral(propagation_literal, state);
    return success;
  }
  return true; // no conflicts occurred during propagation
}

void PropagatorPbSumOutput::ResetCounts() {
  for (auto c : sum_database_->permanent_constraints_) {
    c->current_max_ = c->max_;
    c->set_outputs_.clear();
    //    c->weight_cause_index_.clear();
    //    c->weight_trail_.clear();
    //    c->weight_trail_delimitors_.clear();
  }
}
bool PropagatorPbSumOutput::CheckCounts(SolverState &state,
                                        WatchedPbSumConstraint *constraint) {
  return true;
}
bool PropagatorPbSumOutput::PropagateLiteral(BooleanLiteral true_literal,
                                             SolverState &state) {
  assert(state.assignments_.IsAssignedTrue(true_literal));
  BooleanLiteral false_lit = ~true_literal;
  WatchListPbSum &watch_list_output = sum_database_->watch_list_output_;
  std::vector<WatcherPbSumConstraint> &watchers_false =
      watch_list_output[true_literal];
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
    if (last_index_ != current_index || (!already_partly_done)) {
      int index =
          constraint->lit_to_output_index_[false_lit.ToPositiveInteger()];
      WeightedLiteral wl = constraint->outputs_[index];
      if (wl.weight > constraint->max_) {
        already_partly_done = false;
        last_index_ = current_index;
        continue;
      }
      watchers_false[end_position] = watchers_false[current_index];
      ++end_position;
      if (wl.weight < constraint->current_max_) {
        constraint->set_outputs_.push_back(wl);
        constraint->current_max_ = wl.weight - 1;
      } else {
        already_partly_done = false;
        last_index_ = current_index;
        continue;
      }
    }

    //    assert(constraint->true_count_ >= constraint->weight_trail_.back());
    already_partly_done = false;
    last_index_ = current_index;

    bool success = constraint->current_max_ >= constraint->true_count_;
    if (success) {
      success = PropagateInputs(constraint, state);
    }

    //    success = EnqueTrailWeights(constraint, state);

    if (!success) {
      // conflict
      //    if (!success) {
      // restore remaining watchers
      for (size_t k = current_index + 1; k < watchers_false.size(); ++k) {
        watchers_false[end_position] = watchers_false[current_index];
        ++end_position;
      }
      watchers_false.resize(end_position);
      if (end_position <= last_index_) // TODO how to handle
        last_index_ = end_position - 1;
      failure_constraint_ = constraint;
      //      return false;
      constraint->trigger_count_++;
      trigger_count_++;
      return false;
    }
  }

  watchers_false.resize(end_position);
  if (!next_position_on_trail_to_propagate_it.IsPastTrail())
    next_position_on_trail_to_propagate_it.Next();
  //  assert(CheckCounts(state));
  return true;
}
std::vector<BooleanLiteral>
PropagatorPbSumOutput::GetEncodingCause(SolverState &state,
                                        WatchedPbSumConstraint *constraint) {
  assert(false); // TODO
  return std::vector<BooleanLiteral>();
}
void PropagatorPbSumOutput::PropagateIncremental2(
    SolverState &state, WatchedPbSumConstraint *constraint,
    std::vector<BooleanLiteral> &reason,
    std::vector<BooleanLiteral> &propagate) {
  assert(false); // TODO no longer used?
}
void PropagatorPbSumOutput::AddScheduledEncodings(SolverState &state) {
  while (!add_constraints_.empty()) {
    WatchedPbSumConstraint *constraint = add_constraints_.front();
    add_constraints_.pop();
    if (!constraint->encoder_->EncodingAdded()) {
      constraint->encoder_->Encode(state);
    }
  }
}
void PropagatorPbSumOutput::Synchronise(SolverState &state) {
  // current literal is partly propagated, reduce the counts of the updated
  // constraints.
  if ((!last_propagated_.IsUndefined()) &&
      !next_position_on_trail_to_propagate_it.GetData().IsUndefined() &&
      last_propagated_ == next_position_on_trail_to_propagate_it.GetData()) {
    BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
    if (!sum_database_->watch_list_output_[l].empty()) {
      assert(last_index_ < sum_database_->watch_list_output_[l].size());
      for (int i = 0; i <= last_index_; ++i) {
        WatchedPbSumConstraint *c =
            sum_database_->watch_list_output_[l][i].constraint_;
        assert(c->set_outputs_.back().weight - 1 == c->current_max_);
        assert(c->set_outputs_.back().literal == l);
        c->set_outputs_.pop_back();
        if (c->set_outputs_.empty()) {
          c->current_max_ = c->max_;
        } else {
          c->current_max_ = c->set_outputs_.back().weight - 1;
        }
        assert(c->current_max_ >= 0);
        assert(c->current_max_ <= c->max_);
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
      for (auto wc : sum_database_->watch_list_output_[l]) {
        WatchedPbSumConstraint *c = wc.constraint_;
        if (c->set_outputs_.empty())
          continue;
        assert(c->set_outputs_.back().weight - 1 == c->current_max_);
        if (c->set_outputs_.back().literal != ~l)
          continue;
        c->set_outputs_.pop_back();
        if (c->set_outputs_.empty()) {
          c->current_max_ = c->max_;
        } else {
          c->current_max_ = c->set_outputs_.back().weight - 1;
        }
        assert(c->current_max_ >= 0);
        assert(c->current_max_ <= c->max_);
      }
    }
    assert(next_position_on_trail_to_propagate_it == state.GetTrailEnd());
  }

  //  RecomputeConstraintSums(state, update);
  PropagatorGeneric::Synchronise(state);
  last_propagated_ = BooleanLiteral();
  last_index_ = 0;
}
void PropagatorPbSumOutput::SetTrailIterator(
    TrailList<BooleanLiteral>::Iterator iterator) {
  // set the trail iterator and make sure the the counts stay correct.
  if (iterator.IsFirst()) {
    PropagatorGeneric::SetTrailIterator(iterator);
    ResetCounts();
  } else {

    if (!next_position_on_trail_to_propagate_it.IsPastTrail()) {
      assert(false); // TODO implement
      BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
      for (auto wc : sum_database_->watch_list_true[l]) {
        //        wc.constraint_->true_count_--;
      }
    }

    while (next_position_on_trail_to_propagate_it != iterator) {
      assert(false); // TODO implement
      next_position_on_trail_to_propagate_it.Previous();
      BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
      for (auto wc : sum_database_->watch_list_true[l]) {
        //        wc.constraint_->true_count_--;
      }
    }
  }
  assert(next_position_on_trail_to_propagate_it == iterator);
  last_propagated_ = BooleanLiteral();
  last_index_ = 0;
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
} // namespace Pumpkin