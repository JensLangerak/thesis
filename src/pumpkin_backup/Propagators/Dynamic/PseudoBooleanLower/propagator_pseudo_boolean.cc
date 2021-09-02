//
// Created by jens on 10-12-20.
//

#include "propagator_pseudo_boolean.h"

#include "../../../../logger/logger.h"
#include "../../../Engine/solver_state.h"
#include "../Encoders/incremental_sequential_encoder.h"
#include "../Encoders/totaliser_encoder.h"
#include "reason_pseudo_boolean_constraint.h"
#include "watch_list_pseudo_boolean.h"
#include "watcher_pseudo_boolean_constraint.h"
#include <iostream>
#include <set>
#include <vector>
namespace Pumpkin {

PropagatorPseudoBoolean3::PropagatorPseudoBoolean3(int64_t num_variables)
    : PropagatorDynamic(), pseudo_boolean_database_(num_variables), last_index_(0),
      last_propagated_(BooleanLiteral()) {}
bool PropagatorPseudoBoolean3::PropagateLiteral(BooleanLiteral true_literal,
                                             SolverState &state) {
  assert(state.assignments_.IsAssignedTrue(true_literal));
  WatchListPseudoBoolean3 &watch_list_true = pseudo_boolean_database_.watch_list_true;
  std::vector<WatcherPseudoBooleanConstraint3> &watchers_true =
      watch_list_true[true_literal];
  size_t end_position = 0;
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
//  assert(CheckCounts(state));
  // update all constraints that watch the literal
  for (; current_index < watchers_true.size(); ++current_index) {
    WatchedPseudoBooleanConstraint3 *constraint =
        watchers_true[current_index].constraint_;
    // if the encoding is added, no need to keep track of it anymore
    if (constraint->encoder_->EncodingAdded())
      continue;
    // only update the count the first time that the constraint is triggered.
    // //TODO not sure if the check is still needed
    bool count_updated = false;
    bool prev_already_done = already_partly_done;
    int last_ind = last_index_;
    if (last_index_ != current_index || (!already_partly_done)) {
      count_updated = true;
      constraint->current_sum_value+=watchers_true[current_index].weight_;
    }
    already_partly_done = false;
    last_index_ = current_index;
    int true_count = 0;
//    int count_current=0;
//    int false_count = 0;
//    for (auto wl : constraint->current_literals_) {
//      BooleanLiteral l = wl.literal;
//      if (state.assignments_.IsAssignedTrue(l) &&
//          state.assignments_.GetTrailPosition(l.Variable()) <=
//              state.assignments_.GetTrailPosition(true_literal.Variable()))
//        true_count+= wl.weight;
//
//      if (state.assignments_.IsAssignedTrue(l))
//        count_current += wl.weight;
//    }
//    int count_original = 0;
//    for (auto wl: constraint->original_literals_) {
//      BooleanLiteral l = wl.literal;
//      if (state.assignments_.IsAssignedTrue(l))
//        count_original += wl.weight;
//    }

//    if (count_current != count_original) {
//      int count_added_encoding = 0;
//      for (auto wl:constraint->added_to_encoding_literals_) {
//        if (state.assignments_.IsAssignedTrue(wl.literal))
//          count_added_encoding+=wl.weight;
//      }
//      int count_not_added = 0;
//      for (auto wl:constraint->unencoded_constraint_literals_) {
//        if (state.assignments_.IsAssignedTrue(wl.literal))
//          count_not_added+=wl.weight;
//      }
//      int count_encoding = 0;
//      for (auto wl : constraint->encoded_sum_literals_) {
//        if (state.assignments_.IsAssignedTrue(wl.literal))
//          count_encoding += wl.weight;
//      }
//
//      int test = 2;
//    }
//    int tet = true_literal.VariableIndex();
//        assert(true_count == constraint->current_sum_value);
//    constraint->current_sum_value = true_count;

    true_count = constraint->current_sum_value;

    watchers_true[end_position] = watchers_true[current_index];
    ++end_position;
    // conflict
    if (true_count > constraint->max_) {
//        false_count > constraint->literals_.size() - constraint->min_) {

      // restore remaining watchers
      for (size_t k = current_index + 1; k < watchers_true.size(); ++k) {
        watchers_true[end_position] = watchers_true[k];
        ++end_position;
      }
      watchers_true.resize(end_position);
      failure_constraint_ = constraint;
      //      return false;
      constraint->trigger_count_++;
      trigger_count_++;
//      simple_sat_solver::logger::Logger::Log2("Conflict ID " + std::to_string(constraint->log_id_) + " counts " + std::to_string(constraint->current_sum_value) + " " + std::to_string(trigger_count_));
      if (constraint->encoder_->AddEncodingDynamic()) {
        bool res = AddEncoding(state, constraint);
        //        state.FullReset();
        return res;
      } else {
        return false;
      }
    }

    int slack = constraint->max_ - true_count;

    for (int i = 0; i <constraint->current_literals_.size(); ++i) {
      WeightedLiteral l = constraint->current_literals_[i];
      if (slack >= l.weight)
        break;

      constraint->trigger_count_++;
      trigger_count_++;
//      simple_sat_solver::logger::Logger::Log2("Propagate ID " + std::to_string(constraint->log_id_) + " counts " + std::to_string(constraint->current_sum_value) + " " + std::to_string(trigger_count_));
      if (constraint->encoder_->AddEncodingDynamic() &&
          constraint->encoder_->SupportsIncremental()) {
        assert(false); // TODO
        bool res = PropagateIncremental(state, constraint);
        if (res)
          return true;
      } else {
          if (!state.assignments_.IsAssigned(l.literal)) {
            if (constraint->encoder_->AddEncodingDynamic()) {
              assert(false);//TODO
              assert(!constraint->encoder_->SupportsIncremental());
              bool res = AddEncoding(state, constraint); // TODO skip other literals
              assert(res);
              return true;
            } else {
              // TODO false count
              state.EnqueuePropagatedLiteral(
                  ~l.literal, this, reinterpret_cast<uint64_t>(constraint));
            }
            //          return true;
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
ReasonGeneric *PropagatorPseudoBoolean3::ReasonFailure(SolverState &state) {
  return new ReasonPseudoBooleanConstraint3(failure_constraint_, state);
}
ReasonGeneric *PropagatorPseudoBoolean3::ReasonLiteralPropagation(
    BooleanLiteral propagated_literal, SolverState &state) {
  assert(state.assignments_.GetAssignmentPropagator(
      propagated_literal.Variable()) == this);
  uint64_t code =
      state.assignments_.GetAssignmentCode(propagated_literal.Variable());
  WatchedPseudoBooleanConstraint3 *propagating_constraint =
      reinterpret_cast<WatchedPseudoBooleanConstraint3 *>(code);
  // TODO check if encoding should be added (if called it is not yet added)
  return new ReasonPseudoBooleanConstraint3(propagating_constraint,
                                         propagated_literal, state);
}
ExplanationGeneric *PropagatorPseudoBoolean3::ExplainFailure(SolverState &state) {
  return failure_constraint_->ExplainFailure(state);
}
ExplanationGeneric *PropagatorPseudoBoolean3::ExplainLiteralPropagation(
    BooleanLiteral propagated_literal, SolverState &state) {
  assert(state.assignments_.GetAssignmentPropagator(
      propagated_literal.Variable()) == this);
  uint64_t code =
      state.assignments_.GetAssignmentCode(propagated_literal.Variable());
  WatchedPseudoBooleanConstraint3 *propagating_constraint =
      reinterpret_cast<WatchedPseudoBooleanConstraint3 *>(code);
  // TODO check if encoding should be added (if called it is not yet added)
  return propagating_constraint->ExplainLiteralPropagation(propagated_literal,
                                                           state);
}

void PropagatorPseudoBoolean3::Synchronise(SolverState &state) {

  // current literal is partly propagated, reduce the counts of the updated
  // constraints.
  if ((!last_propagated_.IsUndefined()) && !next_position_on_trail_to_propagate_it.GetData().IsUndefined() &&
      last_propagated_ == next_position_on_trail_to_propagate_it.GetData()) {
    BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
    if (!pseudo_boolean_database_.watch_list_true[l].empty()) {
      assert(last_index_ < pseudo_boolean_database_.watch_list_true[l].size());
      for (int i = 0; i <= last_index_; ++i) {
        pseudo_boolean_database_.watch_list_true[l][i].constraint_->current_sum_value -= pseudo_boolean_database_.watch_list_true[l][i].weight_;
      }
      last_index_ = 0;
      // TODO roll back false
    }
  }

  // move back the iterator and updates the counts
  std::set<WatchedPseudoBooleanConstraint3*> update;
  if (next_position_on_trail_to_propagate_it.IsPastTrail()) {
    while (next_position_on_trail_to_propagate_it != state.GetTrailEnd()) {
      next_position_on_trail_to_propagate_it.Previous();
      BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
      for (auto wc : pseudo_boolean_database_.watch_list_true[l]) {
        update.insert(wc.constraint_);
        wc.constraint_->current_sum_value -= wc.weight_;
      }
    }
    assert(next_position_on_trail_to_propagate_it == state.GetTrailEnd());
    //TODO
//    RecomputeConstraintSums(state, update);

  }

//  RecomputeConstraintSums(state, update);
  PropagatorGeneric::Synchronise(state);
  last_propagated_ = BooleanLiteral();
  last_index_ = 0;
}

bool PropagatorPseudoBoolean3::PropagateOneLiteral(SolverState &state) {
  if (IsPropagationComplete(state) == false) {

    BooleanLiteral propagation_literal =
        *next_position_on_trail_to_propagate_it;
    bool success = PropagateLiteral(propagation_literal, state);
    return success;
  }
  return true; // no conflicts occurred during propagation
}


void PropagatorPseudoBoolean3::ResetCounts() {
  for (auto c : pseudo_boolean_database_.permanent_constraints_) {
    c->current_sum_value = 0;
  }
}


void PropagatorPseudoBoolean3::SetTrailIterator(
    TrailList<BooleanLiteral>::Iterator iterator) {
  // set the trail iterator and make sure the the counts stay correct.
  if (iterator.IsFirst()) {
    PropagatorGeneric::SetTrailIterator(iterator);
    ResetCounts();
  } else {

    if (!next_position_on_trail_to_propagate_it.IsPastTrail()) {
      assert(false); // TODO implement
      BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
      for (auto wc : pseudo_boolean_database_.watch_list_true[l]) {
//        wc.constraint_->true_count_--;
      }
    }


    while (next_position_on_trail_to_propagate_it != iterator) {
      assert(false); // TODO implement
      next_position_on_trail_to_propagate_it.Previous();
      BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
      for (auto wc : pseudo_boolean_database_.watch_list_true[l]) {
//        wc.constraint_->true_count_--;
      }
    }
  }
  assert(next_position_on_trail_to_propagate_it == iterator);
  last_propagated_ = BooleanLiteral();
  last_index_ = 0;
}
bool PropagatorPseudoBoolean3::CheckCounts(SolverState &state) {
//  return true;
  TrailList<BooleanLiteral>::Iterator counter_it = state.GetTrailBegin();
  for (auto c : pseudo_boolean_database_.permanent_constraints_) {
    c->true_count_debug_ = 0;
    //    c->true_log_debug.clear();
  }
  while (counter_it != next_position_on_trail_to_propagate_it) {
    BooleanLiteral l = counter_it.GetData();
    for (auto c : pseudo_boolean_database_.watch_list_true[l]) {
      c.constraint_->true_count_debug_+=c.weight_;
      //      c.constraint_->true_log_debug.emplace_back(c.constraint_->true_count_debug_,
      //      l);
    }
    counter_it.Next();
  }
  for (auto c : pseudo_boolean_database_.permanent_constraints_) {
        assert(c->true_count_debug_ == c->current_sum_value);

  }

  return true;
}

void PropagatorPseudoBoolean3::PropagateIncremental2(SolverState &state, WatchedPseudoBooleanConstraint3 * constraint, std::vector<BooleanLiteral> &reason, std::vector<BooleanLiteral> &propagate) {
  assert(false); // TODO remove?
  assert(constraint != NULL);

  for (int i = 0; i < constraint->current_literals_.size(); ++i) {
    BooleanLiteral l = constraint->current_literals_[i].literal;
    if ((!state.assignments_.IsAssigned(
        l.Variable()))) // && (!constraint->encoder_->IsAdded(l)))
      propagate.push_back(l);
    else if (state.assignments_.IsAssignedTrue(l))
      reason.push_back(l);
  }
}

std::vector<BooleanLiteral>
PropagatorPseudoBoolean3::GetEncodingCause(SolverState &state, WatchedPseudoBooleanConstraint3 *constraint) {
  assert(constraint != NULL);
  std::vector<BooleanLiteral> cause;
  int level_count = 0;
  for (WeightedLiteral wl : constraint->current_literals_) {
    BooleanLiteral l = wl.literal;
    if (state.assignments_.IsAssignedTrue(l)) { // TODO check level
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

struct HeapWeightedLiteral {
  WeightedLiteral l;
  double activity;
  int count;
  HeapWeightedLiteral(WeightedLiteral l, double activity, int count) :l(l), activity(activity), count(count) {};
};

struct HammingDistanceLiteral {
  WeightedLiteral l;
  int distance;
  int count;
  int decisions;
  int compl_dicisions;
  int var_deci;
  int prop;
  HammingDistanceLiteral(WeightedLiteral l, int distance, int count, int decisions, int comp_dec, int var_d, int prop) : l(l), distance(distance), count(count), decisions(decisions), compl_dicisions(comp_dec), var_deci(var_d), prop(prop){};
};
void PropagatorPseudoBoolean3::RecomputeConstraintSums(
    SolverState &state,
    std::set<WatchedPseudoBooleanConstraint3 *> update_constraints) {
  assert(next_position_on_trail_to_propagate_it == state.GetTrailEnd());
  for (auto c : update_constraints) {
    c->current_sum_value = 0;
    for (auto wl : c->current_literals_) {
      if (state.assignments_.IsAssignedTrue(wl.literal)) {
        c->current_sum_value += wl.weight;
        }

    }
  }
}
void PropagatorPseudoBoolean3::AddScheduledEncodings(SolverState& state) {
  for (AddedNode * node : pseudo_boolean_database_.node_database_.scheduled_) {
    if (node->added_)
      continue;
    node->AddEncoding(state);
  }

  pseudo_boolean_database_.node_database_.scheduled_.clear();

}
} // namespace Pumpkin
