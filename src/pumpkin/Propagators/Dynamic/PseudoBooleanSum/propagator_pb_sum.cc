//
// Created by jens on 16-03-21.
//

#include "propagator_pb_sum.h"
#include "../../../Engine/solver_state.h"
#include "../Encoders/generalized_totaliser_sum_nodes.h"
#include "../Encoders/incremental_sequential_encoder.h"
#include "../Encoders/totaliser_encoder.h"
#include "reason_pb_sum_constraint.h"
#include "watch_list_pb_sum.h"
#include <iostream>
#include <stack>
#include <unordered_set>
namespace Pumpkin {

PropagatorPbSum::PropagatorPbSum(int64_t num_variables)
    : PropagatorDynamic(), sum_database_(num_variables), last_index_(0),
      last_propagated_(BooleanLiteral()) {}

ReasonGeneric *PropagatorPbSum::ReasonFailure(SolverState &state) {
  return new ReasonPbSumConstraint(failure_constraint_, state);
}
ReasonGeneric *
PropagatorPbSum::ReasonLiteralPropagation(BooleanLiteral propagated_literal,
                                          SolverState &state) {
  assert(state.assignments_.GetAssignmentPropagator(
             propagated_literal.Variable()) == this);
  uint64_t code =
      state.assignments_.GetAssignmentCode(propagated_literal.Variable());
  WatchedPbSumConstraint *propagating_constraint =
      reinterpret_cast<WatchedPbSumConstraint *>(code);
  // TODO check if encoding should be added (if called it is not yet added)
  return new ReasonPbSumConstraint(propagating_constraint, propagated_literal,
                                   state);
}
ExplanationGeneric *PropagatorPbSum::ExplainFailure(SolverState &state) {
  return failure_constraint_->ExplainFailure(state);
}
ExplanationGeneric *
PropagatorPbSum::ExplainLiteralPropagation(BooleanLiteral propagated_literal,
                                           SolverState &state) {
  assert(state.assignments_.GetAssignmentPropagator(
             propagated_literal.Variable()) == this);
  uint64_t code =
      state.assignments_.GetAssignmentCode(propagated_literal.Variable());
  WatchedPbSumConstraint *propagating_constraint =
      reinterpret_cast<WatchedPbSumConstraint *>(code);
  // TODO check if encoding should be added (if called it is not yet added)
  return propagating_constraint->ExplainLiteralPropagation(propagated_literal,
                                                           state);
}

bool PropagatorPbSum::PropagateOneLiteral(SolverState &state) {
  if (IsPropagationComplete(state) == false) {

    BooleanLiteral propagation_literal =
        *next_position_on_trail_to_propagate_it;
    bool success = PropagateLiteral(propagation_literal, state);
    return success;
  }
  return true; // no conflicts occurred during propagation
}

void PropagatorPbSum::ResetCounts() {
  for (auto c : sum_database_.permanent_constraints_) {
    c->true_count_ = 0;
    c->weight_cause_index_.clear();
    c->weight_trail_.clear();
    c->weight_trail_delimitors_.clear();
  }
}
bool PropagatorPbSum::CheckCounts(SolverState &state,
                                  WatchedPbSumConstraint *constraint) {
  return true;

  std::unordered_set<int> subsum;
  for (WeightedLiteral l : constraint->inputs_) {
    if ((!state.assignments_.IsAssignedTrue(l.literal.Variable())) ||
        (state.assignments_.GetTrailPosition(l.literal.Variable()) >
         state.assignments_.GetTrailPosition(
             state.propagator_pb_sum_.next_position_on_trail_to_propagate_it
                 .GetData()
                 .Variable())))
      continue;
    std::stack<int> new_sums;
    new_sums.push(l.weight);
    for (int i : subsum) {
      int w = i + l.weight;
      new_sums.push(w);
    }
    while (!new_sums.empty()) {
      int w = new_sums.top();
      w = std::min(constraint->max_, w);
      new_sums.pop();
      subsum.insert(w);

      int index = constraint->weight_output_index_map_[w];
      assert(state.assignments_.IsAssignedTrue(constraint->outputs_[index].literal));

    }
  }
  return true;


  int sum = 0;
  for (int i = 0; i < constraint->inputs_.size(); ++i) {
    if ( (!state.assignments_.IsAssignedTrue(constraint->inputs_[i].literal.Variable()))
        || ( state.assignments_.GetTrailPosition(constraint->inputs_[i].literal.Variable()) > state.assignments_.GetTrailPosition(state.propagator_pb_sum_.next_position_on_trail_to_propagate_it.GetData().Variable())))
      continue;
    WeightedLiteral wl = constraint->inputs_[i];
    if (state.assignments_.IsAssignedTrue(wl.literal)) {
      sum += wl.weight;
    }
    int test = std::min(sum, constraint->max_);
    if (test < sum) {
      assert(state.assignments_.IsAssignedFalse(constraint->outputs_.back().literal));
    }
    assert(state.assignments_.IsAssignedTrue(constraint->outputs_[constraint->weight_output_index_map_[test]].literal));

  }
  assert(sum == constraint->true_count_);


}
bool PropagatorPbSum::PropagateLiteral(BooleanLiteral true_literal,
                                       SolverState &state) {
  this->debug_flag = false;
  assert(state.assignments_.IsAssignedTrue(true_literal));
  WatchListPbSum &watch_list_true = sum_database_.watch_list_true;
  std::vector<WatcherPbSumConstraint> &watchers_true =
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
  // update all constraints that watch the literal
  for (; current_index < watchers_true.size(); ++current_index) {
    WatchedPbSumConstraint *constraint =
        watchers_true[current_index].constraint_;
    // if the encoding is added, no need to keep track of it anymore
    if (constraint->encoder_->EncodingAdded())
      continue;
    assert(constraint->true_count_ <= constraint->max_);
    // only update the count the first time that the constraint is triggered.
    // //TODO not sure if the check is still needed
    if (last_index_ != current_index || (!already_partly_done)) {
      UpdateWeightTrail(constraint, true_literal, state);
    }

    assert(constraint->true_count_ >= constraint->weight_trail_.back());
    already_partly_done = false;
    last_index_ = current_index;

    watchers_true[end_position] = watchers_true[current_index];
    ++end_position;
    bool success = true;


//    int sum = 0;
//    for (auto il : constraint->inputs_) {
//      if (state.assignments_.IsAssignedTrue(il.literal)) {
//        sum+=il.weight;
//      }
//    }
//    int lowest_output_w = -1;
//    for (auto ol : constraint->outputs_) {
//      if (ol.weight <= sum) {
//               success = state.EnqueuePropagatedLiteral(
//            ol.literal, this,
//            reinterpret_cast<uint64_t>(constraint));
//               BooleanLiteral enqued = ol.literal;
//        int level = state.assignments_.GetAssignmentLevel(enqued.Variable());
//        bool isdec = state.assignments_.IsDecision(enqued.Variable());
//        if (!success) {
//          std::cout << ol.weight << "  " << (isdec ? "D  " : "P  ") << level
//                    << "   " << state.GetCurrentDecisionLevel() << std::endl;
////          state.value_selector_.UpdatePolarity(enqued.Variable(),
////                                               enqued.IsPositive(), true);
//        }
//      } else {
//        if (state.assignments_.IsAssignedFalse(ol.literal)) {
//          if (lowest_output_w == -1 || ol.weight < lowest_output_w) {
//            lowest_output_w= ol.weight;
//          }
//        }
//      }
//      if (!success)
//        break;
//    }
//    if (success) {
//      for (WeightedLiteral il : constraint->inputs_) {
//        if (!state.assignments_.IsAssigned(il.literal)) {
//          if (il.weight + sum >= lowest_output_w) {
//            success = state.EnqueuePropagatedLiteral(
//                ~il.literal, this, reinterpret_cast<uint64_t>(constraint));
//          }
//        }
//      }
//      assert(success);
//    }

    success = EnqueTrailWeights(constraint, state);


    {
      assert((!success) || CheckCounts(state, constraint));
      int max = ((GeneralizedTotaliserSumNodes *) constraint->encoder_)->max_;
      assert(constraint->true_count_ <= max || (!success));

    }

    if (success) {
      success = PropagateInputs(constraint, state);
      assert(success);
    } else {
    // conflict
//    if (!success) {
      // restore remaining watchers
      for (size_t k = current_index + 1; k < watchers_true.size(); ++k) {
        watchers_true[end_position] = watchers_true[current_index];
        ++end_position;
      }
      watchers_true.resize(end_position);
      if (end_position <= last_index_) //TODO how to handle
        last_index_ = end_position - 1;
      failure_constraint_ = constraint;
      //      return false;
      constraint->trigger_count_++;
      trigger_count_++;
      return false;
    }

//    int true_count = 0;
//    int false_count = 0;
//    std::unordered_set<int> possible_sums;
//    for (WeightedLiteral l : constraint->inputs_) {
//      if (state.assignments_.IsAssignedTrue(l.literal) &&
//          state.assignments_.GetTrailPosition(l.literal.Variable()) <=
//              state.assignments_.GetTrailPosition(true_literal.Variable())) {
//        true_count += l.weight;
//        std::stack<int> new_sums;
//        for (int wi : possible_sums) {
//          new_sums.push(wi + l.weight);
//        }
//        new_sums.push(l.weight);
//        while (!new_sums.empty()) {
//          int wi = new_sums.top();
//          new_sums.pop();
//          possible_sums.insert(wi);
//          if (constraint->weight_output_index_map_.count(wi)>0) {
//            int output_index = constraint->weight_output_index_map_[wi];
//            BooleanLiteral output = constraint->outputs_[output_index].literal;
//            assert(state.assignments_.IsAssignedTrue(output));
//          }
//        }
//      }
//    }
//    assert(true_count == constraint->true_count_);
    //    constraint->true_count_ = true_count;
  }

  watchers_true.resize(end_position);
  if (!next_position_on_trail_to_propagate_it.IsPastTrail())
    next_position_on_trail_to_propagate_it.Next();
  //  assert(CheckCounts(state));
  this->debug_flag = true;
  return true;
}
std::vector<BooleanLiteral>
PropagatorPbSum::GetEncodingCause(SolverState &state,
                                  WatchedPbSumConstraint *constraint) {
  assert(false); // TODO
  return std::vector<BooleanLiteral>();
}
void PropagatorPbSum::PropagateIncremental2(
    SolverState &state, WatchedPbSumConstraint *constraint,
    std::vector<BooleanLiteral> &reason,
    std::vector<BooleanLiteral> &propagate) {
  assert(false); // TODO no longer used?
}
void PropagatorPbSum::AddScheduledEncodings(SolverState &state) {
  while (!add_constraints_.empty()) {
    WatchedPbSumConstraint *constraint = add_constraints_.front();
    add_constraints_.pop();
    if (!constraint->encoder_->EncodingAdded()) {
      constraint->encoder_->Encode(state);
    }
  }
}
void PropagatorPbSum::Synchronise(SolverState &state) {
  // current literal is partly propagated, reduce the counts of the updated
  // constraints.
  if ((!last_propagated_.IsUndefined()) &&
      !next_position_on_trail_to_propagate_it.GetData().IsUndefined() &&
      last_propagated_ == next_position_on_trail_to_propagate_it.GetData()) {
    BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
    if (!sum_database_.watch_list_true[l].empty()) {
      assert(last_index_ < sum_database_.watch_list_true[l].size());
      for (int i = 0; i <= last_index_; ++i) {
        sum_database_.watch_list_true[l][i].constraint_->true_count_ -=
            sum_database_.watch_list_true[l][i].weight_;
        WatchedPbSumConstraint *c =
            sum_database_.watch_list_true[l][i].constraint_;
        int stop = c->weight_trail_delimitors_.back();
        c->weight_trail_delimitors_.pop_back();
        for (int i = c->weight_trail_.size() - 1; i >= stop; --i) {
          int w = c->weight_trail_[i];
          c->weight_cause_index_.erase(w);
          c->weight_trail_.pop_back();
        }
        assert(c->weight_trail_.back() <= c->true_count_);
      }
      last_index_ = 0;
      // TODO roll back false
    }
  }

  // move back the iterator and updates the counts
  std::set<WatchedPbSumConstraint *> update;
  if (next_position_on_trail_to_propagate_it.IsPastTrail()) {
    while (next_position_on_trail_to_propagate_it != state.GetTrailEnd()) {
      next_position_on_trail_to_propagate_it.Previous();
      BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
      for (auto wc : sum_database_.watch_list_true[l]) {
        update.insert(wc.constraint_);
        wc.constraint_->true_count_ -= wc.weight_;

        WatchedPbSumConstraint *c = wc.constraint_;
        int stop = c->weight_trail_delimitors_.back();
        c->weight_trail_delimitors_.pop_back();
        for (int i = c->weight_trail_.size() - 1; i >= stop; --i) {
          int w = c->weight_trail_[i];
          c->weight_cause_index_.erase(w);
          c->weight_trail_.pop_back();
        }
        assert(c->weight_trail_.back() <= c->true_count_);
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
void PropagatorPbSum::SetTrailIterator(
    TrailList<BooleanLiteral>::Iterator iterator) {
  // set the trail iterator and make sure the the counts stay correct.
  if (iterator.IsFirst()) {
    PropagatorGeneric::SetTrailIterator(iterator);
    ResetCounts();
  } else {

    if (!next_position_on_trail_to_propagate_it.IsPastTrail()) {
      assert(false); // TODO implement
      BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
      for (auto wc : sum_database_.watch_list_true[l]) {
        //        wc.constraint_->true_count_--;
      }
    }

    while (next_position_on_trail_to_propagate_it != iterator) {
      assert(false); // TODO implement
      next_position_on_trail_to_propagate_it.Previous();
      BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
      for (auto wc : sum_database_.watch_list_true[l]) {
        //        wc.constraint_->true_count_--;
      }
    }
  }
  assert(next_position_on_trail_to_propagate_it == iterator);
  last_propagated_ = BooleanLiteral();
  last_index_ = 0;
}
void PropagatorPbSum::InitPropagation(WatchedPbSumConstraint *constraint,
                                      SolverState &state) {

  return;
  if (constraint->debug_index == 0) {
    int segseg =4;
  }
  std::unordered_set<int> possible_sums;
  // TODO correct order with cause
  for (WeightedLiteral l : constraint->inputs_) {
    if (state.assignments_.IsAssignedTrue(l.literal)) {
      constraint->true_count_ += l.weight;
      constraint->weight_trail_delimitors_.push_back(
          constraint->weight_trail_.size());
      std::stack<int> new_sums;
      for (int wi : possible_sums) {
        new_sums.push(wi + l.weight);
      }
      new_sums.push(l.weight);
      while (!new_sums.empty()) {
        int wi = new_sums.top();
        new_sums.pop();
        if (wi > constraint->max_)
          wi = constraint->max_;
        if (constraint->weight_cause_index_.count(wi) == 0) {
          possible_sums.insert(wi);
          int output_index = constraint->weight_output_index_map_[wi];
          BooleanLiteral output = constraint->outputs_[output_index].literal;
          constraint->weight_trail_.push_back(wi);
          int input_index =
              constraint->lit_to_input_index_[l.literal.ToPositiveInteger()];
          constraint->weight_cause_index_[wi] = input_index;
          bool success = state.EnqueuePropagatedLiteral(
              output, this, reinterpret_cast<uint64_t>(constraint));
          assert(success);
        }
      }
    }
  }
  if (constraint->debug_index == 0) {
    int segseg =4;
  }
}
bool PropagatorPbSum::PropagateInputs(WatchedPbSumConstraint *constraint,
                                      SolverState &state) {
  bool success = true;
  std::unordered_set<int> forbidden_weights;
  for (auto ow : constraint->outputs_) {
    if (state.assignments_.IsAssignedFalse(ow.literal)) {
      forbidden_weights.insert(ow.weight);
    }
  }
  for (WeightedLiteral il : constraint->inputs_) {
    if (!state.assignments_.IsAssigned(il.literal)) {
      for (int w : constraint->weight_trail_) {

        int sum = il.weight + w;
        if (sum > constraint->max_ || forbidden_weights.count(sum) > 0) {
          success = state.EnqueuePropagatedLiteral(
              ~il.literal, this, reinterpret_cast<uint64_t>(constraint));
        }
      }
    }
  }
  return success;
}
void PropagatorPbSum::UpdateWeightTrail(WatchedPbSumConstraint *constraint,
                                        BooleanLiteral true_literal,
                                        SolverState &state) {
  int index = constraint->lit_to_input_index_[true_literal.ToPositiveInteger()];
  int w = constraint->inputs_[index].weight;
  constraint->true_count_ += w;

  constraint->weight_trail_delimitors_.push_back(
      constraint->weight_trail_.size());
  int stop = constraint->weight_trail_.size();
  for (int i = 0; i < stop; ++i) {
    int nw = constraint->weight_trail_[i] + w;
    if (nw >= constraint->max_) {
      if (constraint->weight_cause_index_.count(constraint->max_) == 0)
        constraint->max_cause_weight = nw;
      nw = constraint->max_;
    }
    if (constraint->weight_cause_index_.count(nw) == 0) {
      constraint->weight_trail_.push_back(nw);
      constraint->weight_cause_index_[nw] = index;
    }
  }
  if (constraint->weight_cause_index_.count(w) == 0) {
    constraint->weight_trail_.push_back(w);
    constraint->weight_cause_index_[w] = index;
  }
}
bool PropagatorPbSum::EnqueTrailWeights(WatchedPbSumConstraint *constraint,
                                        SolverState &state) {
  bool success = true;
  for (int i = constraint->weight_trail_delimitors_.back();
       i < constraint->weight_trail_.size(); ++i) {
    int ow = constraint->weight_trail_[i];
    BooleanLiteral enqued;
    if (constraint->weight_output_index_map_.count(ow) > 0) {
      int oi = constraint->weight_output_index_map_[ow];
      {
        int max = ((GeneralizedTotaliserSumNodes *) constraint->encoder_)->max_;
        if (ow > max) {
          assert(state.assignments_.IsAssignedFalse(constraint->outputs_[oi].literal));
        }
      }
      enqued = constraint->outputs_[oi].literal;
      success = state.EnqueuePropagatedLiteral(
          constraint->outputs_[oi].literal, this,
          reinterpret_cast<uint64_t>(constraint));

    }
    if (!success) {
//        int level = state.assignments_.GetAssignmentLevel(enqued.Variable());
//        bool isdec = state.assignments_.IsDecision(enqued.Variable());
//        state.value_selector_.UpdatePolarity(enqued.Variable(), enqued.IsPositive(), true);
      break;
    }
  }
  return success;
}

} // namespace Pumpkin