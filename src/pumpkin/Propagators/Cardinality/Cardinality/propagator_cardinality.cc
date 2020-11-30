//
// Created by jens on 16-10-20.
//

#include "propagator_cardinality.h"
#include "../../../../logger/logger.h"
#include "../../../Engine/solver_state.h"
#include "../Encoders/incremental_sequential_encoder.h"
#include "../Encoders/totaliser_encoder.h"
#include "reason_cardinality_constraint.h"
#include "watch_list_cardinality.h"
#include <iostream>
namespace Pumpkin {

PropagatorCardinality::PropagatorCardinality(int64_t num_variables)
    : PropagatorDynamic(), cardinality_database_(num_variables), last_index_(0),
      last_propagated_(BooleanLiteral()) {}
bool PropagatorCardinality::PropagateLiteral(BooleanLiteral true_literal,
                                             SolverState &state) {
  assert(state.assignments_.IsAssignedTrue(true_literal));
  WatchListCardinality &watch_list_true = cardinality_database_.watch_list_true;
  std::vector<WatcherCardinalityConstraint> &watchers_true =
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
    WatchedCardinalityConstraint *constraint =
        watchers_true[current_index].constraint_;
    // if the encoding is added, no need to keep track of it anymore
    if (constraint->encoder_->EncodingAdded())
      continue;
    // only update the count the first time that the constraint is triggered.
    // //TODO not sure if the check is still needed
    if (last_index_ != current_index || (!already_partly_done)) {
      constraint->true_count_++;
    }
    already_partly_done = false;
    last_index_ = current_index;
    int true_count = 0;
    int false_count = 0;
//    for (BooleanLiteral l : constraint->literals_) {
//      if (state.assignments_.IsAssignedTrue(l) &&
//          state.assignments_.GetTrailPosition(l.Variable()) <=
//              state.assignments_.GetTrailPosition(true_literal.Variable()))
//        true_count++;
//    }
//        assert(true_count == constraint->true_count_);
//    constraint->true_count_ = true_count;
    true_count = constraint->true_count_;

    watchers_true[end_position] = watchers_true[current_index];
    ++end_position;
    // conflict
    if (true_count > constraint->max_ ||
        false_count > constraint->literals_.size() - constraint->min_) {

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
      simple_sat_solver::logger::Logger::Log2("Conflict ID " + std::to_string(constraint->log_id_) + " counts " + std::to_string(constraint->true_count_) + " " + std::to_string(trigger_count_));
      if (constraint->encoder_->AddEncodingDynamic()) {
        bool res = AddEncoding(state, constraint);
        //        state.FullReset();
        return res;
      } else {
        return false;
      }
    }

    // can propagate
    if (true_count == constraint->max_ ||
        false_count == constraint->literals_.size() - constraint->min_) {
      constraint->trigger_count_++;
      trigger_count_++;
      simple_sat_solver::logger::Logger::Log2("Propagate ID " + std::to_string(constraint->log_id_) + " counts " + std::to_string(constraint->true_count_) + " " + std::to_string(trigger_count_));
      if (constraint->encoder_->AddEncodingDynamic() &&
          constraint->encoder_->SupportsIncremental()) {
        bool res = PropagateIncremental(state, constraint);
        if (res)
        return true;
      } else {
        for (BooleanLiteral l : constraint->literals_) {
          if (!state.assignments_.IsAssigned(l)) {
            if (constraint->encoder_->AddEncodingDynamic()) {
              assert(!constraint->encoder_->SupportsIncremental());
              bool res = AddEncoding(state, constraint);
              assert(res);
              return true;
            } else {
              // TODO false count
              state.EnqueuePropagatedLiteral(
                  ~l, this, reinterpret_cast<uint64_t>(constraint));
            }
            //          return true;
          }
        }
      }
    }
  }

  watchers_true.resize(end_position);
  if (!next_position_on_trail_to_propagate_it.IsPastTrail())
    next_position_on_trail_to_propagate_it.Next();
  //  assert(CheckCounts(state));
  return true;
}
ReasonGeneric *PropagatorCardinality::ReasonFailure(SolverState &state) {
  return new ReasonCardinalityConstraint(failure_constraint_, state);
}
ReasonGeneric *PropagatorCardinality::ReasonLiteralPropagation(
    BooleanLiteral propagated_literal, SolverState &state) {
  assert(state.assignments_.GetAssignmentPropagator(
             propagated_literal.Variable()) == this);
  uint64_t code =
      state.assignments_.GetAssignmentCode(propagated_literal.Variable());
  WatchedCardinalityConstraint *propagating_constraint =
      reinterpret_cast<WatchedCardinalityConstraint *>(code);
  // TODO check if encoding should be added (if called it is not yet added)
  return new ReasonCardinalityConstraint(propagating_constraint,
                                         propagated_literal, state);
}
ExplanationGeneric *PropagatorCardinality::ExplainFailure(SolverState &state) {
  return failure_constraint_->ExplainFailure(state);
}
ExplanationGeneric *PropagatorCardinality::ExplainLiteralPropagation(
    BooleanLiteral propagated_literal, SolverState &state) {
  assert(state.assignments_.GetAssignmentPropagator(
             propagated_literal.Variable()) == this);
  uint64_t code =
      state.assignments_.GetAssignmentCode(propagated_literal.Variable());
  WatchedCardinalityConstraint *propagating_constraint =
      reinterpret_cast<WatchedCardinalityConstraint *>(code);
  // TODO check if encoding should be added (if called it is not yet added)
  return propagating_constraint->ExplainLiteralPropagation(propagated_literal,
                                                           state);
}

void PropagatorCardinality::Synchronise(SolverState &state) {

  // current literal is partly propagated, reduce the counts of the updated
  // constraints.
  if ((!last_propagated_.IsUndefined()) && !next_position_on_trail_to_propagate_it.GetData().IsUndefined() &&
      last_propagated_ == next_position_on_trail_to_propagate_it.GetData()) {
    BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
    if (!cardinality_database_.watch_list_true[l].empty()) {
      assert(last_index_ < cardinality_database_.watch_list_true[l].size());
      for (int i = 0; i <= last_index_; ++i) {
        cardinality_database_.watch_list_true[l][i].constraint_->true_count_--;
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
      for (auto wc : cardinality_database_.watch_list_true[l]) {
        wc.constraint_->true_count_--;
      }
    }
    assert(next_position_on_trail_to_propagate_it == state.GetTrailEnd());
  }
  PropagatorGeneric::Synchronise(state);
  last_propagated_ = BooleanLiteral();
  last_index_ = 0;
}

bool PropagatorCardinality::PropagateOneLiteral(SolverState &state) {
  if (IsPropagationComplete(state) == false) {

    BooleanLiteral propagation_literal =
        *next_position_on_trail_to_propagate_it;
    bool success = PropagateLiteral(propagation_literal, state);
    return success;
  }
  return true; // no conflicts occurred during propagation
}


void PropagatorCardinality::ResetCounts() {
  for (auto c : cardinality_database_.permanent_constraints_) {
    c->true_count_ = 0;
    c->false_count_ = 0;
  }
}


void PropagatorCardinality::SetTrailIterator(
    TrailList<BooleanLiteral>::Iterator iterator) {
  // set the trail iterator and make sure the the counts stay correct.
  if (iterator.IsFirst()) {
    PropagatorGeneric::SetTrailIterator(iterator);
    ResetCounts();
  } else {

    if (!next_position_on_trail_to_propagate_it.IsPastTrail()) {
      BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
      for (auto wc : cardinality_database_.watch_list_true[l]) {
        wc.constraint_->true_count_--;
      }
    }
    while (next_position_on_trail_to_propagate_it != iterator) {
      next_position_on_trail_to_propagate_it.Previous();
      BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
      for (auto wc : cardinality_database_.watch_list_true[l]) {
        wc.constraint_->true_count_--;
      }
    }
  }
  assert(next_position_on_trail_to_propagate_it == iterator);
  last_propagated_ = BooleanLiteral();
  last_index_ = 0;
}
bool PropagatorCardinality::CheckCounts(SolverState &state) {
  return true;
  TrailList<BooleanLiteral>::Iterator counter_it = state.GetTrailBegin();
  for (auto c : cardinality_database_.permanent_constraints_) {
    c->true_count_debug_ = 0;
    //    c->true_log_debug.clear();
  }
  while (counter_it != next_position_on_trail_to_propagate_it) {
    BooleanLiteral l = counter_it.GetData();
    for (auto c : cardinality_database_.watch_list_true[l]) {
      c.constraint_->true_count_debug_++;
      //      c.constraint_->true_log_debug.emplace_back(c.constraint_->true_count_debug_,
      //      l);
    }
    counter_it.Next();
  }
  int miscount = 0;
  int l = 0;
  int g = 0;
  for (auto c : cardinality_database_.permanent_constraints_) {
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

void PropagatorCardinality::PropagateIncremental2(SolverState &state, WatchedCardinalityConstraint * constraint, std::vector<BooleanLiteral> &reason, std::vector<BooleanLiteral> &propagate) {
  assert(constraint != NULL);

  for (int i = 0; i < constraint->literals_.size(); ++i) {
    BooleanLiteral l = constraint->literals_[i];
    if ((!state.assignments_.IsAssigned(
            l.Variable()))) // && (!constraint->encoder_->IsAdded(l)))
      propagate.push_back(l);
    else if (state.assignments_.IsAssignedTrue(l))
      reason.push_back(l);
  }
}

std::vector<BooleanLiteral>
PropagatorCardinality::GetEncodingCause(SolverState &state, WatchedCardinalityConstraint *constraint) {
  assert(constraint != NULL);
    std::vector<BooleanLiteral> cause;
    int level_count = 0;
    for (BooleanLiteral l : constraint->literals_) {
      if (state.assignments_.IsAssignedTrue(l)) {
        cause.push_back(l);
        if (state.assignments_.GetAssignmentLevel(l.Variable()) ==
            state.GetCurrentDecisionLevel())
          level_count++;
      }
    }
    if (cause.size() >= constraint->max_)
      assert(level_count >= 1);
    else
      assert(false);
    assert(cause.size() >= constraint->max_);
    return cause;
}
} // namespace Pumpkin
