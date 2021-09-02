//
// Created by jens on 20-05-21.
//

#ifndef PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_WATCH_LIST_DYNAMIC_H_
#define PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_WATCH_LIST_DYNAMIC_H_
#include "../../Utilities/boolean_literal.h"
#include "../../Utilities/problem_specification.h"
#include <vector>

namespace Pumpkin {
template <class Watched, class Watcher>
class WatchListDynamic {
public:
  explicit WatchListDynamic(uint64_t num_variables);
  void Add(BooleanLiteral literal, int weight, Watched *constraint); //Adds the clause_ to the watch list of the literal. It assumes clause_ copying is forbidden and internally keeps a pointer to "clause_".
  void Add(WeightedLiteral literal, Watched *constraint) { Add(literal.literal, literal.weight, constraint);}
  void Remove(BooleanLiteral literal, Watched *constraint); //Removes the clause_ from the watch list of the literal. It asumes clauses are compared based on the memory locations. Linearly scans through the list until it encounters the clause_, therefore if the index is already know it is better to use the overloaded RemoveClauseFromWatch with an integer as input
  void Remove(BooleanLiteral literal, int constraint_index);//Removes the clause_ at the position 'clause_index' from the watch list of the literal.
  void ClearWatches(BooleanLiteral literal); //removes all watches for the literal

  void Grow(); //increases the number of watched learned_clause_literals by one. Naturally, the newly added variable will contain no watches.

  const std::vector<Watcher>& operator[](BooleanLiteral literal) const; //returns the reference to the vector of clauses that are watched by the literal
  std::vector<Watcher>& operator[](BooleanLiteral literal);

  bool IsLiteralWatchingConstraint(BooleanLiteral literal, const Watched *constraint) const;

protected:
  int ComputeIndexForLiteral(BooleanLiteral literal) const; //computes the position of the literal in the array. This is a simple helper function mostly for clarity.

  std::vector<std::vector<Watcher> > watch_lists_;


};
} // namespace Pumpkin

#endif // PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_WATCH_LIST_DYNAMIC_H_
