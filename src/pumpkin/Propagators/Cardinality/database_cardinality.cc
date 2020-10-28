//
// Created by jens on 16-10-20.
//

#include "database_cardinality.h"
namespace Pumpkin {

DatabaseCardinality::DatabaseCardinality(uint64_t num_vars)
    : watch_list_true(num_vars) {}
WatchedCardinalityConstraint* DatabaseCardinality::AddPermanentConstraint(CardinalityConstraint &constraint, SolverState & state) {
    WatchedCardinalityConstraint * watched = new WatchedCardinalityConstraint(constraint.literals, constraint.min, constraint.max, constraint.encoder);
    permanent_constraints_.push_back(watched);
    AddWatchers(watched);
    return watched;
}
void DatabaseCardinality::AddWatchers(WatchedCardinalityConstraint *constraint) {
  for (BooleanLiteral lit : constraint->literals_) {
    watch_list_true.Add(lit, constraint);
  }

}
DatabaseCardinality::~DatabaseCardinality() {
  for (WatchedCardinalityConstraint *c : permanent_constraints_) {
    for (BooleanLiteral l : c->literals_) {
      watch_list_true.Remove(l, c);
    }
    delete c;
  }

}
} // namespace Pumpkin