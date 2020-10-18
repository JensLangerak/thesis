//
// Created by jens on 16-10-20.
//

#include "database_cardinality.h"
namespace Pumpkin {

DatabaseCardinality::DatabaseCardinality(uint64_t num_vars)
    : watch_list_false(num_vars), watch_list_true(num_vars) {}
WatchedCardinalityConstraint* DatabaseCardinality::AddPermanentConstraint(CardinalityConstraint &constraint, SolverState & state) {
    WatchedCardinalityConstraint * watched = new WatchedCardinalityConstraint(constraint.literals, constraint.min, constraint.max);
    permanent_constraints_.push_back(watched);
    AddWatchers(watched);
    return watched;
}
void DatabaseCardinality::AddPermanentConstraint(
    WatchedCardinalityConstraint *constraint) {
  permanent_constraints_.push_back(constraint);
}
void DatabaseCardinality::AddTemporaryConstraint(
    WatchedCardinalityConstraint *constraint) {
  temporary_constraints_.push_back(constraint);
}
void DatabaseCardinality::AddWatchers(WatchedCardinalityConstraint *constraint) {
  for (BooleanLiteral lit : constraint->literals_) {
    watch_list_true.Add(lit, constraint);
    watch_list_false.Add(~lit, constraint);
  }

}
} // namespace Pumpkin