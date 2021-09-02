//
// Created by jens on 25-05-21.
//

#include "../../../Engine/solver_state.h"
#include "propagator_bottom_layers.h"
#include <iostream>
namespace Pumpkin {

PropagatorBottomLayers::PropagatorBottomLayers(int64_t num_variables)
    : PropagatorDynamic(), pseudo_boolean_database_(num_variables), last_index_(0),
      last_propagated_(BooleanLiteral()) {}
bool PropagatorBottomLayers::PropagateLiteral(BooleanLiteral true_literal,
                                                SolverState &state) {
  assert(state.assignments_.IsAssignedTrue(true_literal));
  WatchListPbBottomLayers &watch_list_true = pseudo_boolean_database_.watch_list_true_;
  std::vector<WatcherPbBottomLayersConstraint> &watchers_true =
      watch_list_true[true_literal];
  size_t current_index = 0;
  // Check if it has partly propagated the literal or not
  bool already_partly_done = false;
  BooleanLiteral last = last_propagated_;
  if (!last_propagated_.IsUndefined() && last_propagated_ == true_literal) {
    current_index = last_index_;
    already_partly_done = true;
  } else {
    last_index_ = 0;
  }
  last_propagated_ = true_literal;
  for (; current_index < watchers_true.size(); ++current_index) {
    WatchedPbBottomLayersConstraint *constraint =
        watchers_true[current_index].constraint_;
    // if the encoding is added, no need to keep track of it anymore
    //    if (constraint->encoder_->EncodingAdded())
    //      continue;
    // only update the count the first time that the constraint is triggered.
    // //TODO not sure if the check is still needed
    bool count_updated = false;
    bool prev_already_done = already_partly_done;
    int last_ind = last_index_;
    if (last_index_ != current_index || (!already_partly_done)) {
      count_updated = true;
      constraint->update_sum_(true_literal,
                              watchers_true[current_index].weight_);
    }
    already_partly_done = false;
    last_index_ = current_index;
    int true_count = 0;

    CheckCount(state, constraint);
    true_count = constraint->current_sum_value_;

    //    watchers_true[end_position] = watchers_true[current_index];
    //    ++end_position;
    // conflict
    if (true_count > constraint->max_) {
      failure_constraint_ = constraint;
      constraint->trigger_count_++;
      trigger_count_++;
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


//  watchers_true.resize(end_position);
  if (!next_position_on_trail_to_propagate_it.IsPastTrail())
    next_position_on_trail_to_propagate_it.Next();
  //  assert(CheckCounts(state));
  return true;
}
ReasonGeneric *PropagatorBottomLayers::ReasonFailure(SolverState &state) {
  return new ReasonPbBottomLayers(failure_constraint_, state);
}
ReasonGeneric *PropagatorBottomLayers::ReasonLiteralPropagation(
    BooleanLiteral propagated_literal, SolverState &state) {
  assert(state.assignments_.GetAssignmentPropagator(
      propagated_literal.Variable()) == this);
  uint64_t code =
      state.assignments_.GetAssignmentCode(propagated_literal.Variable());
  WatchedPbBottomLayersConstraint *propagating_constraint =
      reinterpret_cast<WatchedPbBottomLayersConstraint *>(code);
  // TODO check if encoding should be added (if called it is not yet added)
  return new ReasonPbBottomLayers(propagating_constraint,
                                            propagated_literal, state);
}
ExplanationGeneric *PropagatorBottomLayers::ExplainFailure(SolverState &state) {
  return failure_constraint_->ExplainFailure(state);
}
ExplanationGeneric *PropagatorBottomLayers::ExplainLiteralPropagation(
    BooleanLiteral propagated_literal, SolverState &state) {
  assert(state.assignments_.GetAssignmentPropagator(
      propagated_literal.Variable()) == this);
  uint64_t code =
      state.assignments_.GetAssignmentCode(propagated_literal.Variable());
  WatchedPbBottomLayersConstraint *propagating_constraint =
      reinterpret_cast<WatchedPbBottomLayersConstraint *>(code);
  // TODO check if encoding should be added (if called it is not yet added)
  return propagating_constraint->ExplainLiteralPropagation(propagated_literal,
                                                           state);
}

void PropagatorBottomLayers::Synchronise(SolverState &state) {

  // current literal is partly propagated, reduce the counts of the updated
  // constraints.
  if ((!last_propagated_.IsUndefined()) && (!next_position_on_trail_to_propagate_it.GetData().IsUndefined()) &&
      last_propagated_ == next_position_on_trail_to_propagate_it.GetData()) {
    BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
    if (!pseudo_boolean_database_.watch_list_true_[l].empty()) {
      assert(last_index_ < pseudo_boolean_database_.watch_list_true_[l].size());
      for (int i = 0; i <= last_index_; ++i) {
        auto c = pseudo_boolean_database_.watch_list_true_[l][i].constraint_;
        int node_index = c->lit_to_node_[l.ToPositiveInteger()];
        SumNode& n = c->nodes_[node_index];
        if (n.current_sum_ == pseudo_boolean_database_.watch_list_true_[l][i].weight_) {
          int updated_sum =0;
          for (auto wl : n.sum_literals) {
            if (state.assignments_.IsAssignedTrue(wl.literal) && ((!state.assignments_.IsAssigned(l.Variable())) || state.assignments_.GetTrailPosition(wl.literal.Variable()) < state.assignments_.GetTrailPosition(l.Variable())) &&  wl.weight > updated_sum)
              updated_sum = wl.weight;
          }
          assert(n.current_sum_ > updated_sum);
          int diff = n.current_sum_ - updated_sum;
          n.current_sum_ = updated_sum;
          c->current_sum_value_ -= diff;

        }

      }
      last_index_ = 0;
      // TODO roll back false
    }
  }

  if (next_position_on_trail_to_propagate_it.IsPastTrail()) {
    while (next_position_on_trail_to_propagate_it != state.GetTrailEnd()) {
      next_position_on_trail_to_propagate_it.Previous();
      BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
      for (auto wc : pseudo_boolean_database_.watch_list_true_[l]) {
        auto c = wc.constraint_;
        int node_index = c->lit_to_node_[l.ToPositiveInteger()];
        SumNode& n = c->nodes_[node_index];
        if (n.current_sum_ == wc.weight_) {
          int updated_sum = 0;
          for (auto wl : n.sum_literals) {
            if (state.assignments_.IsAssignedTrue(wl.literal) && ((!state.assignments_.IsAssigned(l.Variable())) || state.assignments_.GetTrailPosition(wl.literal.Variable()) < state.assignments_.GetTrailPosition(l.Variable())) &&
                wl.weight > updated_sum)
              updated_sum = wl.weight;
          }
          assert(n.current_sum_ > updated_sum);
          int diff = n.current_sum_ - updated_sum;
          n.current_sum_ = updated_sum;
          c->current_sum_value_ -= diff;
          assert(c->current_sum_value_ >= 0);
        }
      }
    }
      assert(next_position_on_trail_to_propagate_it == state.GetTrailEnd());
      // TODO
      //    RecomputeConstraintSums(state, update);
  }

//  RecomputeConstraintSums(state, update);
  PropagatorGeneric::Synchronise(state);
  last_propagated_ = BooleanLiteral();
  last_index_ = 0;

}

bool PropagatorBottomLayers::PropagateOneLiteral(SolverState &state) {
  if (IsPropagationComplete(state) == false) {

    BooleanLiteral propagation_literal =
        *next_position_on_trail_to_propagate_it;
    bool success = PropagateLiteral(propagation_literal, state);
    return success;
  }
  return true; // no conflicts occurred during propagation
}


void PropagatorBottomLayers::ResetCounts() {
  for (auto c : pseudo_boolean_database_.permanent_constraints_) {
    c->current_sum_value_ = 0;
    for (SumNode& n : c->nodes_)
      n.current_sum_ = 0;
  }
}


void PropagatorBottomLayers::SetTrailIterator(
    TrailList<BooleanLiteral>::Iterator iterator) {
  // set the trail iterator and make sure the the counts stay correct.
  if (iterator.IsFirst()) {
    PropagatorGeneric::SetTrailIterator(iterator);
    ResetCounts();
  } else {

    if (!next_position_on_trail_to_propagate_it.IsPastTrail()) {
      assert(false); // TODO implement
//      BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
//      for (auto wc : pseudo_boolean_database_.watch_list_true_[l]) {
////        wc.constraint_->true_count_--;
//      }
    }


    while (next_position_on_trail_to_propagate_it != iterator) {
      assert(false); // TODO implement
//      next_position_on_trail_to_propagate_it.Previous();
//      BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
//      for (auto wc : pseudo_boolean_database_.watch_list_true[l]) {
////        wc.constraint_->true_count_--;
//      }
    }
  }
  assert(next_position_on_trail_to_propagate_it == iterator);
  last_propagated_ = BooleanLiteral();
  last_index_ = 0;
}
bool PropagatorBottomLayers::CheckCounts(SolverState &state) {
  return true;
for (auto c : pseudo_boolean_database_.permanent_constraints_) {
  CheckCount(state, c);

}

  return true;
}

void PropagatorBottomLayers::PropagateIncremental2(SolverState &state, WatchedPbBottomLayersConstraint * constraint, std::vector<BooleanLiteral> &reason, std::vector<BooleanLiteral> &propagate) {
  assert(false); // TODO remove?
  assert(constraint != NULL);

//  for (int i = 0; i < constraint->current_literals_.size(); ++i) {
//    BooleanLiteral l = constraint->current_literals_[i].literal;
//    if ((!state.assignments_.IsAssigned(
//        l.Variable()))) // && (!constraint->encoder_->IsAdded(l)))
//      propagate.push_back(l);
//    else if (state.assignments_.IsAssignedTrue(l))
//      reason.push_back(l);
//  }
}

std::vector<BooleanLiteral>
PropagatorBottomLayers::GetEncodingCause(SolverState &state, WatchedPbBottomLayersConstraint *constraint) {
  assert(constraint != NULL);
  assert(false);
  std::vector<BooleanLiteral> cause;
//  int level_count = 0;
//  for (WeightedLiteral wl : constraint->current_literals_) {
//    BooleanLiteral l = wl.literal;
//    if (state.assignments_.IsAssignedTrue(l)) { // TODO check level
//      cause.push_back(l);
//      if (state.assignments_.GetAssignmentLevel(l.Variable()) ==
//          state.GetCurrentDecisionLevel())
//        level_count++;
//    }
//  }
//  if (cause.size() >= constraint->max_)
//    assert(level_count >= 1);
//  else
//    assert(false);
//  assert(cause.size() >= constraint->max_);
  return cause;
}

void PropagatorBottomLayers::AddScheduledEncodings(SolverState& state) {
  return;

}
void PropagatorBottomLayers::CheckCount(SolverState &state,
                                        WatchedPbBottomLayersConstraint *c) {
  int c_sum = 0;
  for (auto n : c->nodes_) {
    int node_value = 0;
    for (auto w : n.sum_literals) {
      int eg=2;
      if (state.assignments_.IsAssignedTrue(w.literal) && state.assignments_.GetTrailPosition(w.literal.Variable()) <= state.assignments_.GetTrailPosition(next_position_on_trail_to_propagate_it.GetData().Variable())) {
        if (w.weight > node_value)
          node_value = w.weight;
      }
    }
    assert(node_value == n.current_sum_);
    c_sum += node_value;
  }
  assert(c->current_sum_value_ == c_sum);
}
} // namespace Pumpkin