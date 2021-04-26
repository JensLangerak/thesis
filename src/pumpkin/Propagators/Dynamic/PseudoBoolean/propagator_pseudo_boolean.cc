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

PropagatorPseudoBoolean2::PropagatorPseudoBoolean2(int64_t num_variables)
    : PropagatorDynamic(), pseudo_boolean_database_(num_variables), last_index_(0),
      last_propagated_(BooleanLiteral()) {}
bool PropagatorPseudoBoolean2::PropagateLiteral(BooleanLiteral true_literal,
                                             SolverState &state) {
  assert(state.assignments_.IsAssignedTrue(true_literal));
  WatchListPseudoBoolean2 &watch_list_true = pseudo_boolean_database_.watch_list_true;
  std::vector<WatcherPseudoBooleanConstraint2> &watchers_true =
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
    WatchedPseudoBooleanConstraint2 *constraint =
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
ReasonGeneric *PropagatorPseudoBoolean2::ReasonFailure(SolverState &state) {
  return new ReasonPseudoBooleanConstraint2(failure_constraint_, state);
}
ReasonGeneric *PropagatorPseudoBoolean2::ReasonLiteralPropagation(
    BooleanLiteral propagated_literal, SolverState &state) {
  assert(state.assignments_.GetAssignmentPropagator(
      propagated_literal.Variable()) == this);
  uint64_t code =
      state.assignments_.GetAssignmentCode(propagated_literal.Variable());
  WatchedPseudoBooleanConstraint2 *propagating_constraint =
      reinterpret_cast<WatchedPseudoBooleanConstraint2 *>(code);
  // TODO check if encoding should be added (if called it is not yet added)
  return new ReasonPseudoBooleanConstraint2(propagating_constraint,
                                         propagated_literal, state);
}
ExplanationGeneric *PropagatorPseudoBoolean2::ExplainFailure(SolverState &state) {
  return failure_constraint_->ExplainFailure(state);
}
ExplanationGeneric *PropagatorPseudoBoolean2::ExplainLiteralPropagation(
    BooleanLiteral propagated_literal, SolverState &state) {
  assert(state.assignments_.GetAssignmentPropagator(
      propagated_literal.Variable()) == this);
  uint64_t code =
      state.assignments_.GetAssignmentCode(propagated_literal.Variable());
  WatchedPseudoBooleanConstraint2 *propagating_constraint =
      reinterpret_cast<WatchedPseudoBooleanConstraint2 *>(code);
  // TODO check if encoding should be added (if called it is not yet added)
  return propagating_constraint->ExplainLiteralPropagation(propagated_literal,
                                                           state);
}

void PropagatorPseudoBoolean2::Synchronise(SolverState &state) {

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
  std::set<WatchedPseudoBooleanConstraint2*> update;
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

bool PropagatorPseudoBoolean2::PropagateOneLiteral(SolverState &state) {
  if (IsPropagationComplete(state) == false) {

    BooleanLiteral propagation_literal =
        *next_position_on_trail_to_propagate_it;
    bool success = PropagateLiteral(propagation_literal, state);
    return success;
  }
  return true; // no conflicts occurred during propagation
}


void PropagatorPseudoBoolean2::ResetCounts() {
  for (auto c : pseudo_boolean_database_.permanent_constraints_) {
    c->current_sum_value = 0;
  }
}


void PropagatorPseudoBoolean2::SetTrailIterator(
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
bool PropagatorPseudoBoolean2::CheckCounts(SolverState &state) {
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

void PropagatorPseudoBoolean2::PropagateIncremental2(SolverState &state, WatchedPseudoBooleanConstraint2 * constraint, std::vector<BooleanLiteral> &reason, std::vector<BooleanLiteral> &propagate) {
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
PropagatorPseudoBoolean2::GetEncodingCause(SolverState &state, WatchedPseudoBooleanConstraint2 *constraint) {
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
void PropagatorPseudoBoolean2::AddScheduledEncodings(SolverState &state) {
  while(!add_constraints_.empty()) {
    WatchedPseudoBooleanConstraint2 * constraint = add_constraints_.front();
    add_constraints_.pop();
    if (!constraint->encoder_->EncodingAdded()) {
      if (constraint->encoder_->SupportsIncremental()) {
        BitStringMap string_map = state.variable_selector_.bit_strings_;
        for (BooleanLiteral l : constraint->add_next_literals_) {
          if (constraint->encoder_->IsAdded(l))
            continue;
          std::vector<HammingDistanceLiteral> candidates;
          std::vector<int> bit_s = string_map.GetKeyValue(l.VariableIndex() - 1);
          for (auto l2 : constraint->unencoded_constraint_literals_) {
            int count_trigger =
                constraint->max_ * constraint->encoder_->add_delay;
            int l2_count =
                constraint->lit_count_[l2.literal.ToPositiveInteger()];
            int l2_decisions =constraint->lit_decisions_[l2.literal.ToPositiveInteger()];
            int l2_decisions2 =constraint->lit_decisions_[(~(l2.literal)).ToPositiveInteger()];
            int var_d = constraint->var_decisions_[l2.literal.VariableIndex()];
            int lit_prop = constraint->lit_prop_[l2.literal.ToPositiveInteger()];
            int count_factor = l2.weight * l2_count;
            if (count_factor * 1.1 > count_trigger) {
              std::vector<int> bit_s2 =
                  string_map.GetKeyValue(l2.literal.VariableIndex() - 1);
              int distance = string_map.HammingDistance(bit_s, bit_s2);
              if (distance > 0.1 * bit_s.size())
                continue;
              HammingDistanceLiteral c =
                  HammingDistanceLiteral(l2, distance, l2_count, l2_decisions, l2_decisions2, var_d, lit_prop);

              candidates.push_back(c);
            }
          }
            std::sort(candidates.begin(), candidates.end(), [](HammingDistanceLiteral a, HammingDistanceLiteral b) {return a.distance < b.distance;}); // TODO perhahps also add weight
            if (candidates.size() > 1 && bit_s.size() > 5) {
//              for (auto c : candidates) {
//                assert(!state.assignments_.IsAssigned(c.l.literal));
//              }

//              for (auto c : candidates) {
//                state.IncreaseDecisionLevel();
//                state.EnqueueDecisionLiteral(c.l.literal);
//                PropagatorGeneric* conflicting_propagator = state.PropagateEnqueued();
//                assert(conflicting_propagator == nullptr);
//                for (auto c2: candidates) {
//                  if (c.l.literal.VariableIndex() == c2.l.literal.VariableIndex())
//                    continue;
//                  if (state.assignments_.IsAssigned(c2.l.literal)) {
//                    int implies = 2;
//                  }
//                }
//                state.Backtrack(0);

//              }

              int s = constraint->original_literals_.size();
              int c_s = candidates.size();
              int seg=2;
            }
            std::vector<BooleanLiteral> add_lits;
            for (auto c : candidates) {
              add_lits.push_back(c.l.literal);
            }
            constraint->encoder_->Encode(state, add_lits);
            constraint->add_next_literals_.clear();
        }

      } else {
        constraint->encoder_->Encode(state);
        constraint->add_next_literals_.clear();
      }


//
//      std::vector<HeapWeightedLiteral> candidates;
//      candidates.reserve(constraint->unencoded_constraint_literals_.size());
//      for (auto l : constraint->unencoded_constraint_literals_) {
//        candidates.push_back(HeapWeightedLiteral(l, state.variable_selector_.heap_.GetKeyValue(l.literal.VariableIndex() - 1), constraint->lit_count_[l.literal.ToPositiveInteger()]));
//      }
//      std::sort(candidates.begin(), candidates.end(), [](HeapWeightedLiteral a, HeapWeightedLiteral b) {return a.activity > b.activity;}); // TODO perhahps also add weight
//      if (constraint->encoder_->SupportsIncremental()) {
//        std::vector<BooleanLiteral> add_lits;
//        double min_activity = -1;
//        double max_acitivity = -1;
//        for (int i = 0; i < candidates.size(); ++i) {
//          HeapWeightedLiteral c = candidates[i];
//          int count_factor = c.count * c.l.weight;
//          int count_trigger = constraint->max_ * constraint->encoder_->add_delay;
//          if (c.activity >= min_activity && c.activity <= max_acitivity && count_factor * 1.1 > count_trigger) {
//            add_lits.push_back(c.l.literal);
//            if (count_factor > count_trigger) {
//              min_activity = c.activity * 0.9;
//              max_acitivity = c.activity * 1.1;
//            }
//          } else if (count_factor > count_trigger) {
//            add_lits.push_back(c.l.literal);
//            min_activity = c.activity * 0.9;
//            max_acitivity = c.activity * 1.1;
//            for (int j = i -1; j >= 0; --j) {
//              HeapWeightedLiteral c2 = candidates[j];
//              int count_factor2 = c2.count * c2.l.weight;
//              if (c2.activity >= min_activity && c2.activity <= max_acitivity) {
//                if (count_factor2 * 1.1 > count_trigger) {
//                  add_lits.push_back(c2.l.literal);
//                }
//              } else {
//                break;
//              }
//            }
//          }
//
//        }
//        constraint->encoder_->Encode(state, add_lits);
//      } else {
//        std::vector<BooleanLiteral> lits;
//        lits.reserve(candidates.size());
//        for (auto c : candidates)
//          lits.push_back(c.l.literal);
//        constraint->encoder_->Encode(state, lits);
//      }
//      constraint->add_next_literals_.clear();

//      if (constraint->encoder_->SupportsIncremental()) {
//        constraint->encoder_->Encode(state, constraint->add_next_literals_);
//        //TODO update constraint
////        for (BooleanLiteral l : constraint->add_next_literals_) {
////          for (int i = 0; i < constraint->unencoded_constraint_literals_.size(); ++i) {
////            WeightedLiteral wl = constraint->unencoded_constraint_literals_[i];
////            if (wl.literal == l) {
////              constraint->unencoded_constraint_literals_.erase(constraint->unencoded_constraint_literals_.begin() + i);
////              constraint->added_to_encoding_literals_.push_back(wl);
////              state.propagator_pseudo_boolean_2_.pseudo_boolean_database_.watch_list_true.Remove(l, constraint);
////            }
////          }
////        }
////        //TODO watch new list
////        std::vector<WeightedLiteral> new_encoded_lits = constraint->encoder_->GetCurrentSumSet();
////        int i = 0;
////        for (WeightedLiteral wl : new_encoded_lits) {
////          if (constraint->encoded_sum_literals_.size() > i && constraint->encoded_sum_literals_[i].literal == wl.literal) {
////            if (constraint->encoded_sum_literals_[i].weight != wl.weight) {
////              state.propagator_pseudo_boolean_2_.pseudo_boolean_database_.watch_list_true.Remove(constraint->encoded_sum_literals_[i].literal, constraint);
////              state.propagator_pseudo_boolean_2_.pseudo_boolean_database_.watch_list_true.Add(wl.literal, wl.weight, constraint);
////              //TODO
////            }
////
////            ++i;
////            continue;
////          }
////          state.propagator_pseudo_boolean_2_.pseudo_boolean_database_.watch_list_true.Add(wl.literal, wl.weight, constraint);
////        }
////        for (; i < constraint->encoded_sum_literals_.size(); ++i)
////          state.propagator_pseudo_boolean_2_.pseudo_boolean_database_.watch_list_true.Remove(constraint->encoded_sum_literals_[i].literal, constraint);
////        constraint->encoded_sum_literals_ = new_encoded_lits;
////
////        constraint->current_literals_ = std::vector<WeightedLiteral>(constraint->unencoded_constraint_literals_);
////        constraint->current_literals_.insert(constraint->current_literals_.begin(), constraint->encoded_sum_literals_.begin(), constraint->encoded_sum_literals_.end());
////        std::sort(constraint->current_literals_.begin(), constraint->current_literals_.end(), [](WeightedLiteral &a, WeightedLiteral &b){return a.weight < b.weight;});
//
//        constraint->add_next_literals_.clear();
//      } else {
//        constraint->encoder_->Encode(state);
//      }
    }

  }
}
void PropagatorPseudoBoolean2::RecomputeConstraintSums(
    SolverState &state,
    std::set<WatchedPseudoBooleanConstraint2 *> update_constraints) {
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
} // namespace Pumpkin
