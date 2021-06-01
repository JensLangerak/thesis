#ifndef WATCH_LIST_H
#define WATCH_LIST_H

#include "../../Basic Data Structures/boolean_literal.h"
#include "two_watched_clause.h"
#include "watcher.h"

#include <vector>

namespace Pumpkin
{

class WatchList
{
public:
	WatchList(int num_variables);//create a watch list for 'num_variables' variables, which includes positive and negative learned_clause_literals for each variable

	void AddClauseToWatch(BooleanLiteral literal, TwoWatchedClause *clause); //Adds the clause_ to the watch list of the literal. It assumes clause_ copying is forbidden and internally keeps a pointer to "clause_".
	void RemoveClauseFromWatch(BooleanLiteral literal, TwoWatchedClause *clause); //Removes the clause_ from the watch list of the literal. It asumes clauses are compared based on the memory locations. Linearly scans through the list until it encounters the clause_, therefore if the index is already know it is better to use the overloaded RemoveClauseFromWatch with an integer as input
	void RemoveClauseFromWatch(BooleanLiteral literal, int clause_index);//Removes the clause_ at the position 'clause_index' from the watch list of the literal.
	void ClearWatches(BooleanLiteral literal); //removes all watches for the literal
	
	void Grow(); //increases the number of watched learned_clause_literals by one. Naturally, the newly added variable will contain no watches.

	bool IsLiteralWatchingClause(BooleanLiteral literal, const TwoWatchedClause *clause) const;

	const std::vector<WatcherClause>& operator[](BooleanLiteral literal) const; //returns the reference to the vector of clauses that are watched by the literal
	std::vector<WatcherClause>& operator[](BooleanLiteral literal);

private:
	int ComputeIndexForLiteral(BooleanLiteral literal) const; //computes the position of the literal in the array. This is a simple helper function mostly for clarity.

	std::vector<std::vector<WatcherClause> > watch_lists_;
};

} //end Pumpkin namespace

#endif // !WATCH_LIST_H