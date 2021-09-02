//
// Created by jens on 10-12-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANLOWER_WATCH_LIST_PSEUDO_BOOLEAN_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANLOWER_WATCH_LIST_PSEUDO_BOOLEAN_H_

#include "../../../Basic Data Structures/boolean_literal.h"
#include "watcher_pseudo_boolean_constraint.h"
#include <cstdint>
namespace Pumpkin {
class WatchListPseudoBoolean3 {
public:
  explicit WatchListPseudoBoolean3(uint64_t num_variables);
  void Add(BooleanLiteral literal, int weight, WatchedPseudoBooleanConstraint3 *constraint); //Adds the clause_ to the watch list of the literal. It assumes clause_ copying is forbidden and internally keeps a pointer to "clause_".
  void Remove(BooleanLiteral literal, WatchedPseudoBooleanConstraint3 *constraint); //Removes the clause_ from the watch list of the literal. It asumes clauses are compared based on the memory locations. Linearly scans through the list until it encounters the clause_, therefore if the index is already know it is better to use the overloaded RemoveClauseFromWatch with an integer as input
  void Remove(BooleanLiteral literal, int constraint_index);//Removes the clause_ at the position 'clause_index' from the watch list of the literal.
  void ClearWatches(BooleanLiteral literal); //removes all watches for the literal

  void Grow(); //increases the number of watched learned_clause_literals by one. Naturally, the newly added variable will contain no watches.

  const std::vector<WatcherPseudoBooleanConstraint3>& operator[](BooleanLiteral literal) const; //returns the reference to the vector of clauses that are watched by the literal
  std::vector<WatcherPseudoBooleanConstraint3>& operator[](BooleanLiteral literal);

  bool IsLiteralWatchingConstraint(BooleanLiteral literal, const WatchedPseudoBooleanConstraint3 *constraint) const;

private:
  int ComputeIndexForLiteral(BooleanLiteral literal) const; //computes the position of the literal in the array. This is a simple helper function mostly for clarity.

  std::vector<std::vector<WatcherPseudoBooleanConstraint3> > watch_lists_;
};
} // namespace Pumpkin
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_WATCH_LIST_PSEUDO_BOOLEAN_H_
