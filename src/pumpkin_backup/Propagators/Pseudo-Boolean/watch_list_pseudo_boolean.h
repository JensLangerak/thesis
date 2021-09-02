#ifndef WATCH_LIST_PSEUDO_BOOLEAN_H
#define WATCH_LIST_PSEUDO_BOOLEAN_H

#include "../../Basic Data Structures/boolean_literal.h"
#include "watched_pseudo_boolean_constraint.h"
#include "watcher_pseudo_boolean.h"

#include <vector>

namespace Pumpkin
{

class WatchListPseudoBoolean
{
public:
	WatchListPseudoBoolean(int num_variables);//create a watch list for 'num_variables' variables, which includes positive and negative learned_clause_literals for each variable

	void Add(BooleanLiteral literal, WatchedPseudoBooleanConstraint *constraint); //Adds the clause_ to the watch list of the literal. It assumes clause_ copying is forbidden and internally keeps a pointer to "clause_".
	void Remove(BooleanLiteral literal, WatchedPseudoBooleanConstraint *constraint); //Removes the clause_ from the watch list of the literal. It asumes clauses are compared based on the memory locations. Linearly scans through the list until it encounters the clause_, therefore if the index is already know it is better to use the overloaded RemoveClauseFromWatch with an integer as input
	void Remove(BooleanLiteral literal, int constraint_index);//Removes the clause_ at the position 'clause_index' from the watch list of the literal.
	void ClearWatches(BooleanLiteral literal); //removes all watches for the literal

	void Grow(); //increases the number of watched learned_clause_literals by one. Naturally, the newly added variable will contain no watches.

	const std::vector<WatcherPseudoBoolean>& operator[](BooleanLiteral literal) const; //returns the reference to the vector of clauses that are watched by the literal
	std::vector<WatcherPseudoBoolean>& operator[](BooleanLiteral literal);

	bool IsLiteralWatchingConstraint(BooleanLiteral literal, const WatchedPseudoBooleanConstraint *constraint) const;

//private:
	int ComputeIndexForLiteral(BooleanLiteral literal) const; //computes the position of the literal in the array. This is a simple helper function mostly for clarity.

	std::vector<std::vector<WatcherPseudoBoolean> > watch_lists_;
};

} //end Pumpkin namespace

#endif // !WATCH_LIST_PSEUDO_BOOLEAN_H
