//
// Created by jens on 10-12-20.
//

#include "watch_list_pseudo_boolean.h"

namespace Pumpkin {

WatchListPseudoBoolean2::WatchListPseudoBoolean2(uint64_t num_variables)
    : watch_lists_(2 + 2 * num_variables) {}
void WatchListPseudoBoolean2::Add(BooleanLiteral literal, int weight,
                               WatchedPseudoBooleanConstraint2 *constraint) {
  watch_lists_[ComputeIndexForLiteral(literal)].push_back(WatcherPseudoBooleanConstraint2(constraint, weight));
}
const std::vector<WatcherPseudoBooleanConstraint2> &
WatchListPseudoBoolean2::operator[](BooleanLiteral literal) const {
  return watch_lists_[ComputeIndexForLiteral(literal)];
}
std::vector<WatcherPseudoBooleanConstraint2> &
WatchListPseudoBoolean2::operator[](BooleanLiteral literal) {
  return watch_lists_[ComputeIndexForLiteral(literal)];
}
void WatchListPseudoBoolean2::Remove(BooleanLiteral literal,
                                  WatchedPseudoBooleanConstraint2 *constraint) {
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
void WatchListPseudoBoolean2::Remove(BooleanLiteral literal,
                                  int constraint_index) {
  int literal_index = ComputeIndexForLiteral(literal);
  watch_lists_[literal_index][constraint_index] = watch_lists_[literal_index].back();
  watch_lists_[literal_index].pop_back();
}
void WatchListPseudoBoolean2::ClearWatches(BooleanLiteral literal) {
  int literal_index = ComputeIndexForLiteral(literal);
  watch_lists_[literal_index].clear();
}
void WatchListPseudoBoolean2::Grow() {
  watch_lists_.push_back(std::vector<WatcherPseudoBooleanConstraint2>());
  watch_lists_.push_back(std::vector<WatcherPseudoBooleanConstraint2>());
}
bool WatchListPseudoBoolean2::IsLiteralWatchingConstraint(
    BooleanLiteral literal,
    const WatchedPseudoBooleanConstraint2 *constraint) const {
  std::vector<WatcherPseudoBooleanConstraint2> watched_constraints = operator[](literal);
  //return std::find(watched_clauses.begin(), watched_clauses.end(), clause) != watched_clauses.end();
  for (WatcherPseudoBooleanConstraint2 &w : watched_constraints)
  {
    if (w.constraint_ == constraint)
    {
      return true;
    }
  }
  return false;
}
int WatchListPseudoBoolean2::ComputeIndexForLiteral(BooleanLiteral literal) const {
  return 2 * literal.Variable().index_ + literal.IsNegative();
}

}