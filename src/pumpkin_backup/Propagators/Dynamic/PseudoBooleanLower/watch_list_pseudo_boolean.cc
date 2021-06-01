//
// Created by jens on 10-12-20.
//

#include "watch_list_pseudo_boolean.h"

namespace Pumpkin {

WatchListPseudoBoolean3::WatchListPseudoBoolean3(uint64_t num_variables)
    : watch_lists_(2 + 2 * num_variables) {}
void WatchListPseudoBoolean3::Add(BooleanLiteral literal, int weight,
                               WatchedPseudoBooleanConstraint3 *constraint) {
  watch_lists_[ComputeIndexForLiteral(literal)].push_back(WatcherPseudoBooleanConstraint3(constraint, weight));
}
const std::vector<WatcherPseudoBooleanConstraint3> &
WatchListPseudoBoolean3::operator[](BooleanLiteral literal) const {
  return watch_lists_[ComputeIndexForLiteral(literal)];
}
std::vector<WatcherPseudoBooleanConstraint3> &
WatchListPseudoBoolean3::operator[](BooleanLiteral literal) {
  return watch_lists_[ComputeIndexForLiteral(literal)];
}
void WatchListPseudoBoolean3::Remove(BooleanLiteral literal,
                                  WatchedPseudoBooleanConstraint3 *constraint) {
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
void WatchListPseudoBoolean3::Remove(BooleanLiteral literal,
                                  int constraint_index) {
  int literal_index = ComputeIndexForLiteral(literal);
  watch_lists_[literal_index][constraint_index] = watch_lists_[literal_index].back();
  watch_lists_[literal_index].pop_back();
}
void WatchListPseudoBoolean3::ClearWatches(BooleanLiteral literal) {
  int literal_index = ComputeIndexForLiteral(literal);
  watch_lists_[literal_index].clear();
}
void WatchListPseudoBoolean3::Grow() {
  watch_lists_.push_back(std::vector<WatcherPseudoBooleanConstraint3>());
  watch_lists_.push_back(std::vector<WatcherPseudoBooleanConstraint3>());
}
bool WatchListPseudoBoolean3::IsLiteralWatchingConstraint(
    BooleanLiteral literal,
    const WatchedPseudoBooleanConstraint3 *constraint) const {
  std::vector<WatcherPseudoBooleanConstraint3> watched_constraints = operator[](literal);
  //return std::find(watched_clauses.begin(), watched_clauses.end(), clause) != watched_clauses.end();
  for (WatcherPseudoBooleanConstraint3 &w : watched_constraints)
  {
    if (w.constraint_ == constraint)
    {
      return true;
    }
  }
  return false;
}
int WatchListPseudoBoolean3::ComputeIndexForLiteral(BooleanLiteral literal) const {
  return 2 * literal.Variable().index_ + literal.IsNegative();
}

}