//
// Created by jens on 16-10-20.
//

#include "propagator_cardinality.h"
#include "../../Engine/solver_state.h"
#include "reason_cardinality_constraint.h"
#include "watch_list_cardinality.h"
namespace Pumpkin {

PropagatorCardinality::PropagatorCardinality(int64_t num_variables)
    : PropagatorGeneric(),
      cardinality_database_(num_variables), last_index_(0),
      last_propagated_(BooleanLiteral()) {}
bool PropagatorCardinality::PropagateLiteral(BooleanLiteral true_literal,
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
  }
  else {
    last_index_ = 0;
  }
  bool t = already_partly_done;
  last_propagated_ = true_literal;
  for (; current_index < watchers_true.size(); ++current_index) {
    WatchedCardinalityConstraint *constraint =
        watchers_true[current_index].constraint_;
    bool inc = false;
    if (last_index_ != current_index || (!already_partly_done)) {
      constraint->true_count_++;
      constraint->true_log.emplace_back(constraint->true_count_, true_literal);
      inc = true;
      }
    already_partly_done = false;
    last_index_ = current_index;
    int true_count = 0;
    int false_count = 0;
    int unassinged_count = 0;
    for (BooleanLiteral l : constraint->literals_) {
      if (!state.assignments_.IsAssigned(l) || state.assignments_.GetTrailPosition(l.Variable()) > state.assignments_.GetTrailPosition(true_literal.Variable())) {
        ++unassinged_count;
      } else if (state.assignments_.IsAssignedTrue(l)) {
        ++true_count;
      } else {
        ++false_count;
      }
    }
    // TODO handle with propagation
//    constraint->true_count_ = true_count;
    assert(constraint->true_count_ == true_count);
    constraint->false_count_ = false_count;
//    int true_count = constraint->true_count_;
//    int false_count = constraint->false_count_;

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
      return false;
    }
    if (true_count == constraint->max_ ||
        false_count == constraint->literals_.size() - constraint->min_) {
      for (BooleanLiteral l : constraint->literals_) {
        if (!state.assignments_.IsAssigned(l)) {
          uint64_t code = reinterpret_cast<uint64_t>(
              constraint); // the code will simply be a pointer to the
                           // propagating clause
          if (true_count == constraint->max_)
            state.EnqueuePropagatedLiteral(~l, this, code);
          else
            state.EnqueuePropagatedLiteral(l, this, code);
          return true;
        }
      }
    }


  }

  watchers_true.resize(end_position);
  next_position_on_trail_to_propagate_++;
  next_position_on_trail_to_propagate_it.Next();
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
  return propagating_constraint->ExplainLiteralPropagation(propagated_literal,
                                                           state);
}

void PropagatorCardinality::Synchronise(SolverState &state) {

  //TODO do this before the trail is rolled back
  BooleanLiteral l2;
  if ((!last_propagated_.IsUndefined()) && last_propagated_ == next_position_on_trail_to_propagate_it.GetData()) {
    BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
    l2 = l;
    if (!cardinality_database_.watch_list_true[l].empty()) {
    assert(last_index_ < cardinality_database_.watch_list_true[l].size());
    for (int i = 0; i <= last_index_; ++i) {

      assert(cardinality_database_.watch_list_true[l][i].constraint_->true_log.back().count == cardinality_database_.watch_list_true[l][i].constraint_->true_count_);
      assert(cardinality_database_.watch_list_true[l][i].constraint_->true_log.back().lit == l);
      cardinality_database_.watch_list_true[l][i].constraint_->true_count_--;
      cardinality_database_.watch_list_true[l][i].constraint_->true_log.pop_back();
      assert(cardinality_database_.watch_list_true[l][i].constraint_->true_log.empty() && cardinality_database_.watch_list_true[l][i].constraint_->true_count_ == 0
           || cardinality_database_.watch_list_true[l][i].constraint_->true_log.back().count == cardinality_database_.watch_list_true[l][i].constraint_->true_count_);
    }
    last_index_ = 0;
    //TODO roll back false
  }}
//  if (next_position_on_trail_to_propagate_ > state.GetNumberOfAssignedVariables()) {
    if (!state.assignments_.IsAssigned(next_position_on_trail_to_propagate_it.GetData())) {
      assert(next_position_on_trail_to_propagate_ >= state.GetNumberOfAssignedVariables());
    while (next_position_on_trail_to_propagate_it != state.GetTrailEnd()) {
      next_position_on_trail_to_propagate_it.Previous();
      --next_position_on_trail_to_propagate_;
      BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
      for (auto wc : cardinality_database_.watch_list_true[l]) {
        assert(wc.constraint_->true_log.back().count == wc.constraint_->true_count_);
        assert(wc.constraint_->true_log.back().lit == l);
        wc.constraint_->true_count_--;
        wc.constraint_->true_log.pop_back();
        assert(wc.constraint_->true_log.empty() && wc.constraint_->true_count_ == 0 ||wc.constraint_->true_log.back().count == wc.constraint_->true_count_);
      }
      for (auto wc : cardinality_database_.watch_list_false[~l]) {
        //      wc.constraint_->false_count_--;
      }
    }
    assert(next_position_on_trail_to_propagate_ == state.GetNumberOfAssignedVariables());
    assert(next_position_on_trail_to_propagate_it == state.GetTrailEnd());
  } else {
    assert(next_position_on_trail_to_propagate_ < state.GetNumberOfAssignedVariables());
  }
  PropagatorGeneric::Synchronise(state);
  last_propagated_ = BooleanLiteral();
  last_index_ = 0;
}

bool PropagatorCardinality::PropagateOneLiteral(SolverState &state) {
  if (IsPropagationComplete(state) == false) {

    BooleanLiteral propagation_literal = state.GetLiteralFromTrailAtPosition(
        next_position_on_trail_to_propagate_);
        BooleanLiteral propagation_literal2 = *next_position_on_trail_to_propagate_it;
        assert(propagation_literal == propagation_literal2);
    bool success = PropagateLiteral(propagation_literal2, state);
    if (success == false) {
      return false;
    }
  }
  return true; // no conflicts occurred during propagation
}
} // namespace Pumpkin
