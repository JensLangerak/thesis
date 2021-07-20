//
// Created by jens on 06-07-21.
//

#include "database_bottom_layers.h"
#include "watched_bottom_layers_constraint.h"

namespace Pumpkin {

DatabaseBottomLayers::DatabaseBottomLayers(uint64_t num_vars) : watch_list_true_(num_vars){}
WatchedBottomLayersConstraint *DatabaseBottomLayers::AddPermanentConstraint(
    PseudoBooleanConstraint &constraint, SolverState &state) {
  WatchedBottomLayersConstraint * watched = new WatchedBottomLayersConstraint(constraint);
  watched->EncodeBottomLayers(state, 1);

  permanent_constraints_.push_back(watched);
  AddWatchers(watched);
  return watched;
}
void DatabaseBottomLayers::AddWatchers(
    WatchedBottomLayersConstraint *constraint) {
  for (auto n : constraint->nodes_) {
    for (WeightedLiteral lit : n.sum_literals) {
      watch_list_true_.Add(lit, constraint);
    }
  }
}
}