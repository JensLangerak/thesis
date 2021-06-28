//
// Created by jens on 07-06-21.
//

#include "database_pb_pairs.h"
#include "../../../Utilities/problem_specification.h"
#include "../../../Engine/solver_state.h";
#include "watched_pb_pairs_constraint.h";

namespace Pumpkin {

DatabasePbPairs::DatabasePbPairs(uint64_t num_vars) : watch_list_true_(num_vars){}
WatchedPbPairsConstraint *
DatabasePbPairs::AddPermanentConstraint(PseudoBooleanConstraint &constraint,
                                        SolverState &state) {
  WatchedPbPairsConstraint * watched = new WatchedPbPairsConstraint(constraint);
  watched->pairs_database_ = &pairs_database_;
  permanent_constraints_.push_back(watched);
  AddWatchers(watched);
  return watched;
}
DatabasePbPairs::~DatabasePbPairs() {
  for (auto *c : permanent_constraints_) {
    for (WeightedLiteral l : c->intput_liters_) {
      watch_list_true_.Remove(l.literal, c);
    }
    delete c;
  }
}
void DatabasePbPairs::AddWatchers(WatchedPbPairsConstraint *constraint) {
  for (WeightedLiteral lit : constraint->intput_liters_) {
    watch_list_true_.Add(lit.literal, lit.weight, constraint);
  }
}
}