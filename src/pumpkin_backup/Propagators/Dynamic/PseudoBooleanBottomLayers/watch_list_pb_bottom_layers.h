//
// Created by jens on 18-05-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANBOTTOMLAYERS_WATCH_LIST_PB_BOTTOM_LAYERS_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANBOTTOMLAYERS_WATCH_LIST_PB_BOTTOM_LAYERS_H_


#include "../../../Basic Data Structures/boolean_literal.h"
#include "watched_pb_bottom_layers_constraint.h"
#include "watcher_pb_bottom_layers_constraint.h"
#include <vector>
namespace Pumpkin {
class WatchListPbBottomLayers {
public:
  explicit WatchListPbBottomLayers(uint64_t num_variables);
  void Add(WeightedLiteral literal,
           WatchedPbBottomLayersConstraint *
               constraint); // Adds the clause_ to the watch list of the literal. It asPbSumes clause_ copying is forbidden and internally keeps a pointer to "clause_".
  void Remove(
      BooleanLiteral literal,
      WatchedPbBottomLayersConstraint *
          constraint); // Removes the clause_ from the watch list of the literal. It aPbSumes clauses are compared based on the memory locations. Linearly scans through the list until it encounters the clause_, therefore if the index is already know it is better to use the overloaded RemoveClauseFromWatch with an integer as input
  void Remove(BooleanLiteral literal,
              int constraint_index); // Removes the clause_ at the position 'clause_index' from the watch list of the literal.
  void
  ClearWatches(BooleanLiteral literal); // removes all watches for the literal

  void
  Grow(); // increases the number of watched learned_clause_literals by one. Naturally, the newly added variable will contain no watches.

  const std::vector<WatcherPbBottomLayersConstraint> &operator[](
      BooleanLiteral literal) const; // returns the reference to the vector of clauses that are watched by the literal
  std::vector<WatcherPbBottomLayersConstraint> &
  operator[](BooleanLiteral literal);

  bool IsLiteralWatchingConstraint(
      BooleanLiteral literal,
      const WatchedPbBottomLayersConstraint *constraint) const;

private:
  int ComputeIndexForLiteral(BooleanLiteral literal)
      const; // computes the position of the literal in the array. This is a simple helper function mostly for clarity.

  std::vector<std::vector<WatcherPbBottomLayersConstraint>> watch_lists_;
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANBOTTOMLAYERS_WATCH_LIST_PB_BOTTOM_LAYERS_H_
