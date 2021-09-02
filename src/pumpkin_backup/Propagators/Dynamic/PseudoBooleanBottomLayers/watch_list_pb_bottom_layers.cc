//
// Created by jens on 18-05-21.
//

#include "watch_list_pb_bottom_layers.h"


namespace Pumpkin {

WatchListPbBottomLayers::WatchListPbBottomLayers(uint64_t num_variables)
    : watch_lists_(2 + 2 * num_variables) {}
void WatchListPbBottomLayers::Add(WeightedLiteral literal,
                         WatchedPbBottomLayersConstraint *constraint) {
  watch_lists_[ComputeIndexForLiteral(literal.literal)].push_back(WatcherPbBottomLayersConstraint(constraint, literal.weight));
}
const std::vector<WatcherPbBottomLayersConstraint> &
WatchListPbBottomLayers::operator[](BooleanLiteral literal) const {
  return watch_lists_[ComputeIndexForLiteral(literal)];
}
std::vector<WatcherPbBottomLayersConstraint> &
WatchListPbBottomLayers::operator[](BooleanLiteral literal) {
  return watch_lists_[ComputeIndexForLiteral(literal)];
}
void WatchListPbBottomLayers::Remove(BooleanLiteral literal,
                            WatchedPbBottomLayersConstraint *constraint) {
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
void WatchListPbBottomLayers::Remove(BooleanLiteral literal,
                            int constraint_index) {
  int literal_index = ComputeIndexForLiteral(literal);
  watch_lists_[literal_index][constraint_index] = watch_lists_[literal_index].back();
  watch_lists_[literal_index].pop_back();
}
void WatchListPbBottomLayers::ClearWatches(BooleanLiteral literal) {
  int literal_index = ComputeIndexForLiteral(literal);
  watch_lists_[literal_index].clear();
}
void WatchListPbBottomLayers::Grow() {
  watch_lists_.push_back(std::vector<WatcherPbBottomLayersConstraint>());
  watch_lists_.push_back(std::vector<WatcherPbBottomLayersConstraint>());
}
bool WatchListPbBottomLayers::IsLiteralWatchingConstraint(
    BooleanLiteral literal,
    const WatchedPbBottomLayersConstraint *constraint) const {
  std::vector<WatcherPbBottomLayersConstraint> watched_constraints = operator[](literal);
  //return std::find(watched_clauses.begin(), watched_clauses.end(), clause) != watched_clauses.end();
  for (WatcherPbBottomLayersConstraint &w : watched_constraints)
  {
    if (w.constraint_ == constraint)
    {
      return true;
    }
  }
  return false;
}
int WatchListPbBottomLayers::ComputeIndexForLiteral(BooleanLiteral literal) const {
  return 2 * literal.Variable().index_ + literal.IsNegative();
}

}