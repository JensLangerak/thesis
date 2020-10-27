//
// Created by jens on 16-10-20.
//

#include "propagator_cardinality2.h"
#include "../../Engine/solver_state.h"
#include "Encoders/totaliser_encoder.h"
#include "reason_cardinality_constraint.h"
#include "watch_list_cardinality.h"
#include <iostream>
//TODO remove file
namespace Pumpkin {

PropagatorCardinality2::PropagatorCardinality2(int64_t num_variables)
    : PropagatorGeneric(), cardinality_database_(num_variables), last_index_(0),
      last_propagated_(BooleanLiteral()) {}
bool PropagatorCardinality2::PropagateLiteral(BooleanLiteral true_literal,
                                             SolverState &state) {
  assert(state.assignments_.IsAssignedTrue(true_literal));
  WatchListCardinality &watch_list_true = cardinality_database_.watch_list_true;
  std::vector<WatcherCardinalityConstraint> &watchers_true =
      watch_list_true[true_literal];
  size_t end_position = 0;
  size_t current_index = 0;
  bool already_partly_done = false;
  if (!last_propagated_.IsUndefined() && last_propagated_ == true_literal) {
    current_index = last_index_;
    already_partly_done = true;
  } else {
    last_index_ = 0;
  }
  last_propagated_ = true_literal;
  for (; current_index < watchers_true.size(); ++current_index) {
    WatchedCardinalityConstraint *constraint =
        watchers_true[current_index].constraint_;
    if (last_index_ != current_index || (!already_partly_done)) {
      constraint->true_count_++;
//      constraint->true_log.emplace_back(constraint->true_count_, true_literal);
    }
    already_partly_done = false;
    last_index_ = current_index;
    int true_count = 0;
    int false_count = 0;
    int unassinged_count = 0;

    true_count = constraint->true_count_;

    watchers_true[end_position] = watchers_true[current_index];
    ++end_position;
    if (true_count > constraint->max_ ||
        false_count > constraint->literals_.size() - constraint->min_) {
      // restore remaining watchers
      for (size_t k = current_index + 1; k < watchers_true.size(); ++k) {
        watchers_true[end_position] = watchers_true[current_index];
        ++end_position;
      }
      watchers_true.resize(end_position);
      failure_constraint_ = constraint;
      this->trigger_count_++;
      constraint->trigger_count_++;
      return false;
    }
    if (true_count == constraint->max_ ||
        false_count == constraint->literals_.size() - constraint->min_) {
      constraint->trigger_count_++;
      this->trigger_count_++;
      for (BooleanLiteral l : constraint->literals_) {
        if (!state.assignments_.IsAssigned(l)) {
           state.EnqueuePropagatedLiteral(~l, this,  reinterpret_cast<uint64_t>(constraint));
        }
      }
    }
  }

  watchers_true.resize(end_position);
  //  next_position_on_trail_to_propagate_++;
  next_position_on_trail_to_propagate_it.Next();
  return true;
}
ReasonGeneric *PropagatorCardinality2::ReasonFailure(SolverState &state) {
  return new ReasonCardinalityConstraint(failure_constraint_, state);
}
ReasonGeneric *PropagatorCardinality2::ReasonLiteralPropagation(
    BooleanLiteral propagated_literal, SolverState &state) {
  assert(state.assignments_.GetAssignmentPropagator(
             propagated_literal.Variable()) == this);
  uint64_t code =
      state.assignments_.GetAssignmentCode(propagated_literal.Variable());
  WatchedCardinalityConstraint *propagating_constraint =
      reinterpret_cast<WatchedCardinalityConstraint *>(code);
  return new ReasonCardinalityConstraint(propagating_constraint,
                                         propagated_literal, state);
}
ExplanationGeneric *PropagatorCardinality2::ExplainFailure(SolverState &state) {
  return failure_constraint_->ExplainFailure(state);
}
ExplanationGeneric *PropagatorCardinality2::ExplainLiteralPropagation(
    BooleanLiteral propagated_literal, SolverState &state) {
  assert(state.assignments_.GetAssignmentPropagator(
             propagated_literal.Variable()) == this);
  uint64_t code =
      state.assignments_.GetAssignmentCode(propagated_literal.Variable());
  WatchedCardinalityConstraint *propagating_constraint =
      reinterpret_cast<WatchedCardinalityConstraint *>(code);
  return propagating_constraint->ExplainLiteralPropagation(propagated_literal,
                                                           state);
}

void PropagatorCardinality2::Synchronise(SolverState &state) {

  // TODO do this before the trail is rolled back
  if ((!last_propagated_.IsUndefined()) &&
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
  if (next_position_on_trail_to_propagate_it.IsPastTrail()) {
    while (next_position_on_trail_to_propagate_it != state.GetTrailEnd()) {
      next_position_on_trail_to_propagate_it.Previous();
      BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
      for (auto wc : cardinality_database_.watch_list_true[l]) {
        wc.constraint_->true_count_--;
      }
    }
    assert(next_position_on_trail_to_propagate_it == state.GetTrailEnd());
  } else {
  }
  PropagatorGeneric::Synchronise(state);
  last_propagated_ = BooleanLiteral();
  last_index_ = 0;
}

bool PropagatorCardinality2::PropagateOneLiteral(SolverState &state) {
  if (IsPropagationComplete(state) == false) {

    BooleanLiteral propagation_literal2 =
        *next_position_on_trail_to_propagate_it;
    bool success = PropagateLiteral(propagation_literal2, state);
    if (success == false) {
      return false;
    }
  }
  return true; // no conflicts occurred during propagation
}
void PropagatorCardinality2::ResetCounts() {
  for (auto c : cardinality_database_.permanent_constraints_) {
    c->true_count_ = 0;
    c->false_count_ = 0;
//    c->true_log.clear();
  }
}

void PropagatorCardinality2::SetTrailIterator(
    TrailList<BooleanLiteral>::Iterator iterator) {
  if (iterator.IsFirst()) {
    PropagatorGeneric::SetTrailIterator(iterator);
    ResetCounts();
  } else {
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
bool PropagatorCardinality2::CheckCounts(SolverState &state) {
  TrailList<BooleanLiteral>::Iterator counter_it = state.GetTrailBegin();
  for (auto c : cardinality_database_.permanent_constraints_) {
    c->true_count_debug_ = 0;
    }
  while (counter_it != next_position_on_trail_to_propagate_it) {
    BooleanLiteral l = counter_it.GetData();
    for (auto c : cardinality_database_.watch_list_true[l]) {
      c.constraint_->true_count_debug_++;
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
} // namespace Pumpkin
