//
// Created by jens on 16-10-20.
//

#include "watch_list_cardinality.h"
#include <assert.h>
namespace Pumpkin {

WatchListCardinality::WatchListCardinality(uint64_t num_variables)
    : watch_lists_(2 + 2 * num_variables) {}
void WatchListCardinality::Add(BooleanLiteral literal,
                               WatchedCardinalityConstraint *constraint) {
  watch_lists_[ComputeIndexForLiteral(literal)].push_back(WatcherCardinalityConstraint(constraint));
}
const std::vector<WatcherCardinalityConstraint> &
WatchListCardinality::operator[](BooleanLiteral literal) const {
  return watch_lists_[ComputeIndexForLiteral(literal)];
}
std::vector<WatcherCardinalityConstraint> &
WatchListCardinality::operator[](BooleanLiteral literal) {
  return watch_lists_[ComputeIndexForLiteral(literal)];
}
void WatchListCardinality::Remove(BooleanLiteral literal,
                                  WatchedCardinalityConstraint *constraint) {
  int literal_index = ComputeIndexForLiteral(literal);
  for (int clause_index = 0; clause_index < watch_lists_[literal_index].size(); clause_index++)
  {
    if (watch_lists_[literal_index][clause_index].constraint_ == constraint)
    {
      watch_lists_[literal_index][clause_index] = watch_lists_[literal_index].back();
      watch_lists_[literal_index].pop_back();
      return;
    }
  }
  assert(1 == 2); //something must have been removed, should never reach this
}
void WatchListCardinality::Remove(BooleanLiteral literal,
                                  int constraint_index) {
  int literal_index = ComputeIndexForLiteral(literal);
  watch_lists_[literal_index][constraint_index] = watch_lists_[literal_index].back();
  watch_lists_[literal_index].pop_back();
}
void WatchListCardinality::ClearWatches(BooleanLiteral literal) {
  int literal_index = ComputeIndexForLiteral(literal);
  watch_lists_[literal_index].clear();
}
void WatchListCardinality::Grow() {
  watch_lists_.push_back(std::vector<WatcherCardinalityConstraint>());
  watch_lists_.push_back(std::vector<WatcherCardinalityConstraint>());
}
bool WatchListCardinality::IsLiteralWatchingConstraint(
    BooleanLiteral literal,
    const WatchedCardinalityConstraint *constraint) const {
  std::vector<WatcherCardinalityConstraint> watched_constraints = operator[](literal);
  //return std::find(watched_clauses.begin(), watched_clauses.end(), clause) != watched_clauses.end();
  for (WatcherCardinalityConstraint &w : watched_constraints)
  {
    if (w.constraint_ == constraint)
    {
      return true;
    }
  }
  return false;
}
int WatchListCardinality::ComputeIndexForLiteral(BooleanLiteral literal) const {
  return 2 * literal.Variable().index_ + literal.IsNegative();
}

}