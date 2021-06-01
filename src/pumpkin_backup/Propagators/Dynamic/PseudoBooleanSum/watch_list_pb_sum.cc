//
// Created by jens on 16-03-21.
//

#include "watch_list_pb_sum.h"

namespace Pumpkin {

WatchListPbSum::WatchListPbSum(uint64_t num_variables)
    : watch_lists_(2 + 2 * num_variables) {}
void WatchListPbSum::Add(WeightedLiteral literal,
                       WatchedPbSumConstraint *constraint) {
  watch_lists_[ComputeIndexForLiteral(literal.literal)].push_back(WatcherPbSumConstraint(constraint, literal.weight));
}
const std::vector<WatcherPbSumConstraint> &
WatchListPbSum::operator[](BooleanLiteral literal) const {
  return watch_lists_[ComputeIndexForLiteral(literal)];
}
std::vector<WatcherPbSumConstraint> &
WatchListPbSum::operator[](BooleanLiteral literal) {
  return watch_lists_[ComputeIndexForLiteral(literal)];
}
void WatchListPbSum::Remove(BooleanLiteral literal,
                          WatchedPbSumConstraint *constraint) {
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
//  assert(1 == 2); //something must have been removed, should never reach this
}
void WatchListPbSum::Remove(BooleanLiteral literal,
                          int constraint_index) {
  int literal_index = ComputeIndexForLiteral(literal);
  watch_lists_[literal_index][constraint_index] = watch_lists_[literal_index].back();
  watch_lists_[literal_index].pop_back();
}
void WatchListPbSum::ClearWatches(BooleanLiteral literal) {
  int literal_index = ComputeIndexForLiteral(literal);
  watch_lists_[literal_index].clear();
}
void WatchListPbSum::Grow() {
  watch_lists_.push_back(std::vector<WatcherPbSumConstraint>());
  watch_lists_.push_back(std::vector<WatcherPbSumConstraint>());
}
bool WatchListPbSum::IsLiteralWatchingConstraint(
    BooleanLiteral literal,
    const WatchedPbSumConstraint *constraint) const {
  std::vector<WatcherPbSumConstraint> watched_constraints = operator[](literal);
  //return std::find(watched_clauses.begin(), watched_clauses.end(), clause) != watched_clauses.end();
  for (WatcherPbSumConstraint &w : watched_constraints)
  {
    if (w.constraint_ == constraint)
    {
      return true;
    }
  }
  return false;
}
int WatchListPbSum::ComputeIndexForLiteral(BooleanLiteral literal) const {
  return 2 * literal.Variable().index_ + literal.IsNegative();
}

}