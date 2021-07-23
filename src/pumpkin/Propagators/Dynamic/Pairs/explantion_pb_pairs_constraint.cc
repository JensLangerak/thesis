//
// Created by jens on 07-06-21.
//

#include "explantion_pb_pairs_constraint.h"
#include "../../../Engine/solver_state.h"
#include <unordered_set>
#include <chrono>

#define REPLACEMODE_CAUSE
namespace Pumpkin {
using std::chrono::high_resolution_clock;
using std::chrono::duration;
//double ExplanationPbPairsConstraint::time_used_ = 0.0;
//double ExplanationPbPairsConstraint::avg_pair_size_ = 0.0;
//int ExplanationPbPairsConstraint::replace_count_ =0;

void ExplanationPbPairsConstraint::InitExplanationPbPairsConstraint(
    WatchedPbPairsConstraint *constraint, SolverState &state,
    ExplanationDynamicConstraint *explanation) {
  assert(constraint != nullptr);
  assert(constraint->current_sum_value_ > constraint->max_);
  //  constraint->UpdateConflictCount(state);
  auto lits_ = std::vector<BooleanLiteral>();
  // Check if the min or max constraint is violated
  bool select_value = constraint->current_sum_value_ > constraint->max_;
  int sum = 0;
#ifdef REPLACEMODE_CAUSE
  std::vector<WeightedLiteral> causes;
#else
  std::unordered_set<BooleanLiteral> causes;
#endif
  for (auto wl : constraint->intput_liters_) {
    BooleanLiteral l = wl.literal;
    if (state.assignments_.IsAssigned(l) &&
        state.assignments_.GetAssignment(l) == select_value) {
#ifdef REPLACEMODE_CAUSE
      causes.push_back(wl);
#else
      causes.insert(l);
#endif
      sum += wl.weight;
    }
  }


//  auto t1 = high_resolution_clock::now();
#ifdef REPLACEMODE_CAUSE
  ReplaceLits(causes, state, constraint->pairs_database_, lits_);
#else
  ReplaceLits2(causes, constraint, state, constraint->pairs_database_, lits_);
#endif

//  auto t2 = high_resolution_clock::now();
//  duration<double, std::milli> ms_double = t2 - t1;
//  auto time_used_old = time_used_;
//  time_used_ += ms_double.count();

  // TODO not sure if >= should be possible or that is should be ==
  if (sum < constraint->max_) {
    assert(sum >= constraint->max_);
  }
  explanation->Init(lits_);
}
void ExplanationPbPairsConstraint::InitExplanationPbPairsConstraint(
    WatchedPbPairsConstraint *constraint, SolverState &state,
    BooleanLiteral propagated_literal,
    ExplanationDynamicConstraint *explanation) {
  assert(constraint != nullptr);
  assert(state.assignments_.IsAssignedTrue(propagated_literal));
  // check if true for the minimum value is propagated or false for the upper bound.
  bool propagated_value = true;
  int l_w = 0;
  WeightedLiteral propagated;
  for (auto wl : constraint->intput_liters_) {
    BooleanLiteral l = wl.literal;
    if (l == propagated_literal || ~l == propagated_literal) {
      propagated_value = l == propagated_literal;
      l_w = wl.weight;
      propagated = wl;
      break;
    }
  }
  assert(propagated_value == false);

  // get the cause for the propagation
  bool cause_value = !propagated_value;
  int propagation_level =
      state.assignments_.GetTrailPosition(propagated_literal.Variable());
  auto lits_ = std::vector<BooleanLiteral>();
  int sum = 0;
#ifdef REPLACEMODE_CAUSE
  std::vector<WeightedLiteral> causes;
#else
  std::unordered_set<BooleanLiteral> causes;
#endif
  for (auto wl : constraint->intput_liters_) {
    BooleanLiteral l = wl.literal;
    if (state.assignments_.IsAssigned(l) &&
        state.assignments_.GetTrailPosition(l.Variable()) < propagation_level &&
        state.assignments_.GetAssignment(l) == cause_value) {

#ifdef REPLACEMODE_CAUSE
      causes.push_back(wl);
#else
      causes.insert(l);
#endif
      sum += wl.weight;
    }
  }
//  auto t1 = high_resolution_clock::now();
#ifdef REPLACEMODE_CAUSE
  ReplaceLits(causes, state, constraint->pairs_database_, lits_);
#else
  ReplaceLits2(causes, constraint, state, constraint->pairs_database_, lits_);
#endif
  assert(sum + l_w > constraint->max_);
//  auto t2 = high_resolution_clock::now();
//  duration<double, std::milli> ms_double = t2 - t1;
//  auto time_used_old = time_used_;
//  time_used_ += ms_double.count();
  explanation->Init(lits_);
}
void ExplanationPbPairsConstraint::ReplaceLits2Wrapper(
    std::vector<WeightedLiteral> causes, WatchedPbPairsConstraint *constraint,
    SolverState &state, PairsDatabase *pairs_database,
    std::vector<BooleanLiteral> &lits_) {

  std::unordered_set<BooleanLiteral> unreplaced_lits;
  for (auto l : causes)
    unreplaced_lits.insert(l.literal);

  ReplaceLits2(unreplaced_lits, constraint, state, pairs_database, lits_);

}

std::unordered_set<BooleanLiteral> ExplanationPbPairsConstraint::unreplaced_lits;
std::unordered_set<BooleanLiteral> ExplanationPbPairsConstraint::replaced_lits;
void ExplanationPbPairsConstraint::ReplaceLits2(
    std::unordered_set<BooleanLiteral> & causes, WatchedPbPairsConstraint *constraint,
    SolverState &state, PairsDatabase *pairs_database,
    std::vector<BooleanLiteral> &lits_) {
//  std::unordered_set<BooleanLiteral> unreplaced_lits;
//  unreplaced_lits.reserve(causes.size());
//  std::unordered_set<BooleanLiteral> replaced_lits;
//  replaced_lits.reserve(causes.size());
unreplaced_lits.clear();
replaced_lits.clear();
  for (auto l : causes) {
    if (replaced_lits.find(l) != replaced_lits.end())
      continue;
    if (pairs_database->pairs_.find(l) == pairs_database->pairs_.end()) {
      unreplaced_lits.insert(l);
      continue;
    }
    auto pairs = pairs_database->pairs_[l];
    BooleanLiteral candidate = BooleanLiteral();

//    double sp = pairs.size();
//    avg_pair_size_ = (avg_pair_size_ * replace_count_ + sp) / (replace_count_ + 1);
//    avg_pair_size_ = avg_pair_size_ * 0.9 + 0.1 * sp;
//    replace_count_++;

      for (auto p : pairs) {
        BooleanLiteral other = p.first;
        if (causes.find(other) != causes.end() &&
            replaced_lits.find(other) == replaced_lits.end()) {
          candidate = other;
          if (p.second->added_)
            break;
        }
    }
    if (candidate.code_ == 0) {
      unreplaced_lits.insert(l);
      continue;
    }


//    unreplaced_lits.erase(candidate);
//    unreplaced_lits.erase(l);
    replaced_lits.insert(candidate);
    replaced_lits.insert(l);
    if (pairs[candidate]->added_) {
      lits_.push_back(pairs[candidate]->outputs_.back().literal);
    } else {
      lits_.push_back(l);
      lits_.push_back(candidate);
    }
  }

  BooleanLiteral prev = BooleanLiteral();
  for (auto l : unreplaced_lits) {
    lits_.push_back(l);
//    if (pairs_database->pairs_.find(l) != pairs_database->pairs_.end()) {
//      if (pairs_database->pairs_[l].size() > 2)
//        continue;
//    }
    if (prev.code_ == 0) {
      prev = l;
    } else {
      WeightedLiteral p = WeightedLiteral(prev, constraint->lit_weights_[prev]);
      WeightedLiteral c = WeightedLiteral(l, constraint->lit_weights_[l]);
      pairs_database->CreateNode(p, c, state);
      prev = BooleanLiteral();
    }
  }
}

void ExplanationPbPairsConstraint::ReplaceLits(
    std::vector<WeightedLiteral> causes, SolverState &state,
    PairsDatabase *pairs_database, std::vector<BooleanLiteral> &lits_) {

  std::vector<WeightedLiteral> unreplaced_lits;
  std::unordered_set<int> handled_indices;
  std::unordered_map<BooleanLiteral, int> lit_to_index;
  for (int i = 0; i < lits_.size(); ++i)
    lit_to_index[lits_[i]] = i;

  for (int i = 0; i < causes.size(); ++i) {
    if (handled_indices.count(i) > 0)
      continue;
    WeightedLiteral l = causes[i];
    if (pairs_database->pairs_.count(l.literal) == 0) {
      unreplaced_lits.push_back(l);
      continue;
    }
    auto pairs = pairs_database->pairs_[l.literal];
    if (pairs.empty()) {
      unreplaced_lits.push_back(l);
      continue;
    }
    int pairss = pairs.size();
    int causs = causes.size();
    int unscheduled_index = -1;
    bool replaced = false;
    //    for (auto p : pairs) {
    //      BooleanLiteral l2 = p.first;
    //      if (lit_to_index.count(l2) > 0) {
    //        auto node = p.second;
    //        int index = lit_to_index[l2];
    //        if (handled_indices.count(index) > 0)
    //          continue;
    //        if (node->added_) {
    //
    //          int assignment_pos = state.assignments_.GetTrailPosition(l.literal.Variable());
    //          assignment_pos= std::max(assignment_pos, state.assignments_.GetTrailPosition(l2.Variable())); if (state.assignments_.GetTrailPosition(node->outputs_.back().literal.Variable()) < assignment_pos)
    //            continue;
    //          int assignment_lev = state.assignments_.GetAssignmentLevel(l.literal.Variable());
    //          assignment_lev= std::max(assignment_lev, state.assignments_.GetAssignmentLevel(l2.Variable())); int aaf = state.assignments_.GetAssignmentLevel(node->outputs_.back().literal.Variable());
    //          assert(state.assignments_.GetAssignmentLevel(node->outputs_.back().literal.Variable()) == assignment_lev);
    //
    //          lits_.push_back(node->outputs_.back().literal);
    ////          lits_.push_back(l.literal);
    ////          lits_.push_back(l2.literal);
    //          handled_indices.insert(index);
    //          replaced = true;
    //          assert(state.assignments_.IsAssignedTrue(node->inputs_[1].literal));
    //          assert(state.assignments_.IsAssignedTrue(node->inputs_[0].literal));
    //          assert(state.assignments_.IsAssigned(node->outputs_.back().literal));
    //          assert(state.assignments_.IsAssignedTrue(node->outputs_.back().literal));
    //          break;
    //        } else {
    //          unscheduled_index = index;
    //        }
    //      }
    //    }

    for (int j = i + 1; j < causes.size(); ++j) {
      if (handled_indices.count(j) > 0)
        continue;
      WeightedLiteral l2 = causes[j];
      if (pairs.count(l2.literal) > 0) {
        auto node = pairs[l2.literal];
        assert(node->outputs_.back().weight ==
               node->inputs_[0].weight + node->inputs_[1].weight);
        if (node->added_) {
          assert(
              state.assignments_.IsAssignedTrue(node->outputs_.back().literal));
          int assignment_pos =
              state.assignments_.GetTrailPosition(l.literal.Variable());
          assignment_pos = std::max(
              assignment_pos,
              state.assignments_.GetTrailPosition(l2.literal.Variable()));
          if (state.assignments_.GetTrailPosition(
                  node->outputs_.back().literal.Variable()) < assignment_pos)
            continue;
          int assignment_lev =
              state.assignments_.GetAssignmentLevel(l.literal.Variable());
          assignment_lev = std::max(
              assignment_lev,
              state.assignments_.GetAssignmentLevel(l2.literal.Variable()));
          int aaf = state.assignments_.GetAssignmentLevel(
              node->outputs_.back().literal.Variable());
          assert(state.assignments_.GetAssignmentLevel(
                     node->outputs_.back().literal.Variable()) ==
                 assignment_lev);

          lits_.push_back(node->outputs_.back().literal);
          //          lits_.push_back(l.literal);
          //          lits_.push_back(l2.literal);
          handled_indices.insert(j);
          replaced = true;
          assert(state.assignments_.IsAssignedTrue(node->inputs_[1].literal));
          assert(state.assignments_.IsAssignedTrue(node->inputs_[0].literal));
          assert(state.assignments_.IsAssigned(node->outputs_.back().literal));
          assert(
              state.assignments_.IsAssignedTrue(node->outputs_.back().literal));
          break;
        } else {
          unscheduled_index = j;
        }
      }
    }
    if ((!replaced) && unscheduled_index > -1) {
      lits_.push_back(l.literal);
      lits_.push_back(causes[unscheduled_index].literal);
      handled_indices.insert(unscheduled_index);
    } else if (!replaced) {
      unreplaced_lits.push_back(l);
    }
  }

  for (int i = 0; i + 1 < unreplaced_lits.size(); i += 2) {
    pairs_database->CreateNode(unreplaced_lits[i], unreplaced_lits[i + 1],
                               state);
    lits_.push_back(unreplaced_lits[i].literal);
    lits_.push_back(unreplaced_lits[i + 1].literal);
  }
  if (unreplaced_lits.size() % 2 == 1) {
    lits_.push_back(unreplaced_lits.back().literal);
  }
}
void ExplanationPbPairsConstraint::InitLits(int64_t size) {
//  unreplaced_lits.reserve(size);
//  replaced_lits.reserve(size);
}
}
