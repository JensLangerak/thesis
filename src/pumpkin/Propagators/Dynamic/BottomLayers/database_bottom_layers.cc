//
// Created by jens on 06-07-21.
//

#include "database_bottom_layers.h"
#include "watched_bottom_layers_constraint.h"
#include "../../../Engine/solver_state.h"

namespace Pumpkin {

DatabaseBottomLayers::DatabaseBottomLayers(uint64_t num_vars) : watch_list_true_(num_vars){}
WatchedBottomLayersConstraint *DatabaseBottomLayers::AddPermanentConstraint(
    PseudoBooleanConstraint &constraint, SolverState &state) {
  WatchedBottomLayersConstraint * watched = new WatchedBottomLayersConstraint(constraint);
  watched->EncodeBottomLayers(state, 1);

  permanent_constraints_.push_back(watched);
  AddWatchers(watched, state);
  assert(watched->CountCorrect(state, state.trail_.back()));

  return watched;
}
void DatabaseBottomLayers::AddWatchers(
    WatchedBottomLayersConstraint *constraint, SolverState &state) {
  for (auto n : constraint->nodes_) {
    for (WeightedLiteral lit : n.sum_literals) {
      watch_list_true_.Add(lit, constraint);

      if (state.assignments_.IsAssignedTrue(lit.literal)) {
        constraint->UpdateSum(lit.literal, lit.weight, state);
        constraint->UpdateDecisionLevelSum(state);
      }
    }
  }
  if (constraint->current_sum_value_ > constraint->max_) {
    throw "Error";
  }

  int slack = constraint->max_ - constraint->current_sum_value_;

  for (int i = 0; i < constraint->nodes_.size(); ++i) {
    for (WeightedLiteral l : constraint->nodes_[i].sum_literals) {
      if (!state.assignments_.IsAssigned(l.literal)) {
        int wdiff = l.weight - constraint->nodes_[i].current_sum_;
        if (slack < wdiff) {
          state.EnqueueDecisionLiteral(~l.literal);
        }
      }
    }
  }
}
}