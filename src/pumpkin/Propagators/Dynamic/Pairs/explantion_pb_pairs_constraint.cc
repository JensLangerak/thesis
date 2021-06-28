//
// Created by jens on 07-06-21.
//

#include "explantion_pb_pairs_constraint.h"
#include "../../../Engine/solver_state.h"
#include <unordered_set>
namespace Pumpkin {
void ExplanationPbPairsConstraint::InitExplanationPbPairsConstraint(
    WatchedPbPairsConstraint *constraint, SolverState &state,
    ExplanationDynamicConstraint *explanation) {
  assert(constraint!= nullptr);
  assert(constraint->current_sum_value_ > constraint->max_);
//  constraint->UpdateConflictCount(state);
  auto lits_ = std::vector<BooleanLiteral>();
  // Check if the min or max constraint is violated
  bool select_value = constraint->current_sum_value_ > constraint->max_;
  int sum = 0;
  std::vector<WeightedLiteral> causes;
  for (auto wl : constraint->intput_liters_) {
    BooleanLiteral l = wl.literal;
    if (state.assignments_.IsAssigned(l) &&
        state.assignments_.GetAssignment(l) == select_value) {
      causes.push_back(wl);
      sum += wl.weight;
    }
  }

  ReplaceLits(causes, state, constraint->pairs_database_, lits_);


  // TODO not sure if >= should be possible or that is should be ==
  if (sum < constraint ->max_) {
    assert(sum >= constraint->max_);
  }
  explanation->Init(lits_);


}
void ExplanationPbPairsConstraint::InitExplanationPbPairsConstraint(
    WatchedPbPairsConstraint *constraint, SolverState &state,
    BooleanLiteral propagated_literal,
    ExplanationDynamicConstraint *explanation) {
  assert(constraint!= nullptr);
  assert(state.assignments_.IsAssignedTrue(propagated_literal));
  // check if true for the minimum value is propagated or false for the upper bound.
  bool propagated_value = true;
  int l_w = 0;
  WeightedLiteral propagated;
  for (auto wl : constraint->intput_liters_) {
    BooleanLiteral l =wl.literal;
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
  int propagation_level = state.assignments_.GetTrailPosition(propagated_literal.Variable());
  auto lits_ = std::vector<BooleanLiteral>();
  int sum = 0;
  std::vector<WeightedLiteral> causes;
  for (auto wl : constraint->intput_liters_) {
    BooleanLiteral l = wl.literal;
    if (state.assignments_.IsAssigned(l) && state.assignments_.GetTrailPosition(l.Variable()) < propagation_level &&
        state.assignments_.GetAssignment(l) == cause_value) {
      causes.push_back(wl);
      sum += wl.weight;
    }
  }
  ReplaceLits(causes, state, constraint->pairs_database_, lits_);
  assert(sum + l_w > constraint->max_);
  lits_.push_back(~propagated_literal); //TODO not sure if correct?
  // TODO trim lits if to many
  explanation->Init(lits_);

}
void ExplanationPbPairsConstraint::ReplaceLits(
    std::vector<WeightedLiteral> causes, SolverState &state, PairsDatabase * pairs_database,
    std::vector<BooleanLiteral> &lits_) {

  std::vector<WeightedLiteral> unreplaced_lits;
  std::unordered_set<int> handled_indices;

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
    int unscheduled_index = -1;
    bool replaced = false;
    for (int j = i + 1; j < causes.size(); ++j) {
      if (handled_indices.count(j) > 0)
        continue;
      WeightedLiteral l2 = causes[j];
      if (pairs.count(l2.literal) > 0) {
        auto node = pairs[l2.literal];
        assert(node->outputs_.back().weight == node->inputs_[0].weight + node->inputs_[1].weight);
        if (node->added_) {
          assert(state.assignments_.IsAssignedTrue(node->outputs_.back().literal));
          int assignment_pos = state.assignments_.GetTrailPosition(l.literal.Variable());
          assignment_pos= std::max(assignment_pos, state.assignments_.GetTrailPosition(l2.literal.Variable()));
          if (state.assignments_.GetTrailPosition(node->outputs_.back().literal.Variable()) < assignment_pos)
            continue;
          int assignment_lev = state.assignments_.GetAssignmentLevel(l.literal.Variable());
          assignment_lev= std::max(assignment_lev, state.assignments_.GetAssignmentLevel(l2.literal.Variable()));
          int aaf = state.assignments_.GetAssignmentLevel(node->outputs_.back().literal.Variable());
          assert(state.assignments_.GetAssignmentLevel(node->outputs_.back().literal.Variable()) == assignment_lev);

          lits_.push_back(node->outputs_.back().literal);
//          lits_.push_back(l.literal);
//          lits_.push_back(l2.literal);
          handled_indices.insert(j);
          replaced = true;
          assert(state.assignments_.IsAssignedTrue(node->inputs_[1].literal));
          assert(state.assignments_.IsAssignedTrue(node->inputs_[0].literal));
          assert(state.assignments_.IsAssigned(node->outputs_.back().literal));
          assert(state.assignments_.IsAssignedTrue(node->outputs_.back().literal));
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

  for (int i = 0; i + 1 < unreplaced_lits.size() ; i+=2) {
    pairs_database->CreateNode(unreplaced_lits[i], unreplaced_lits[i+1], state);
    lits_.push_back(unreplaced_lits[i].literal);
    lits_.push_back(unreplaced_lits[i+1].literal);
  }
  if (unreplaced_lits.size() % 2 == 1) {
    lits_.push_back(unreplaced_lits.back().literal);
  }
}
}
