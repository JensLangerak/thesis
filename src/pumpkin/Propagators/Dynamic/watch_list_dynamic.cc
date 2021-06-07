//
// Created by jens on 20-05-21.
//

#include "watch_list_dynamic.h"
#include "Pairs/watched_pb_pairs_constraint.h"
#include "PseudoBoolean/watched_pseudo_boolean_constraint_2.h"
#include "watcher_dynamic_constraint.h"
namespace Pumpkin {

template <class Watched, class Watcher>
WatchListDynamic<Watched, Watcher>::WatchListDynamic(uint64_t num_variables)
    : watch_lists_(2 + 2 * num_variables) {}
template <class Watched, class Watcher>
void WatchListDynamic<Watched, Watcher>::Add(BooleanLiteral literal, int weight,
                                             Watched *constraint) {
  watch_lists_[ComputeIndexForLiteral(literal)].push_back(
      Watcher(constraint, weight));
}
template <class Watched, class Watcher>
const std::vector<Watcher> &
WatchListDynamic<Watched, Watcher>::operator[](BooleanLiteral literal) const {
  return watch_lists_[ComputeIndexForLiteral(literal)];
}
template <class Watched, class Watcher>
std::vector<Watcher> &
WatchListDynamic<Watched, Watcher>::operator[](BooleanLiteral literal) {
  return watch_lists_[ComputeIndexForLiteral(literal)];
}
template <class Watched, class Watcher>
void WatchListDynamic<Watched, Watcher>::Remove(BooleanLiteral literal,
                                                Watched *constraint) {
  int literal_index = ComputeIndexForLiteral(literal);
  for (int clause_index = 0; clause_index < watch_lists_[literal_index].size();
       clause_index++) {
    if (watch_lists_[literal_index][clause_index].constraint_ == constraint) {
      watch_lists_[literal_index][clause_index] =
          watch_lists_[literal_index].back();
      watch_lists_[literal_index].pop_back();
      return;
    }
  }
  //  assert(1 == 2); //something must have been removed, should never reach
  //  this
}
template <class Watched, class Watcher>
void WatchListDynamic<Watched, Watcher>::Remove(BooleanLiteral literal,
                                                int constraint_index) {
  int literal_index = ComputeIndexForLiteral(literal);
  watch_lists_[literal_index][constraint_index] =
      watch_lists_[literal_index].back();
  watch_lists_[literal_index].pop_back();
}
template <class Watched, class Watcher>
void WatchListDynamic<Watched, Watcher>::ClearWatches(BooleanLiteral literal) {
  int literal_index = ComputeIndexForLiteral(literal);
  watch_lists_[literal_index].clear();
}
template <class Watched, class Watcher>
void WatchListDynamic<Watched, Watcher>::Grow() {
  watch_lists_.push_back(std::vector<Watcher>());
  watch_lists_.push_back(std::vector<Watcher>());
}
template <class Watched, class Watcher>
bool WatchListDynamic<Watched, Watcher>::IsLiteralWatchingConstraint(
    BooleanLiteral literal, const Watched *constraint) const {
  std::vector<Watcher> watched_constraints = operator[](literal);
  // return std::find(watched_clauses.begin(), watched_clauses.end(), clause) !=
  // watched_clauses.end();
  for (Watcher &w : watched_constraints) {
    if (w.constraint_ == constraint) {
      return true;
    }
  }
  return false;
}
template <class Watched, class Watcher>
int WatchListDynamic<Watched, Watcher>::ComputeIndexForLiteral(
    BooleanLiteral literal) const {
  return 2 * literal.Variable().index_ + literal.IsNegative();
}

template class WatchListDynamic<WatchedPseudoBooleanConstraint2, WatcherDynamicConstraint<WatchedPseudoBooleanConstraint2>>;
template class WatchListDynamic<WatchedPbPairsConstraint, WatcherDynamicConstraint<WatchedPbPairsConstraint>>;
} // namespace Pumpkin
