//
// Created by jens on 07-06-21.
//

#include "database_pb_pairs.h"

namespace Pumpkin {

DatabasePbPairs::DatabasePbPairs(uint64_t num_vars) : watch_list_true_(num_vars){}
WatchedPbPairsConstraint *
DatabasePbPairs::AddPermanentConstraint(PseudoBooleanConstraint &constraint,
                                        SolverState &state) {
  WatchedPbPairsConstraint * watched = new WatchedPbPairsConstraint(constraint.literals, constraint.coefficients, constraint.right_hand_side);
  permanent_constraints_.push_back(watched);
  AddWatchers(watched);
  return watched;
}
DatabasePbPairs::~DatabasePbPairs() {
  for (auto *c : permanent_constraints_) {
    for (WeightedLiteral l : c->current_literals_) {
      watch_list_true_.Remove(l.literal, c);
    }
    delete c;
  }
}
void DatabasePbPairs::AddWatchers(WatchedPbPairsConstraint *constraint) {
  for (WeightedLiteral lit : constraint->current_literals_) {
    watch_list_true_.Add(lit.literal, lit.weight, constraint);
  }
}
}