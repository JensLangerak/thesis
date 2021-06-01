#ifndef TWO_WATCHED_CLAUSE_H
#define TWO_WATCHED_CLAUSE_H

#include "../../Utilities/boolean_variable.h"
#include "../../Utilities/boolean_literal.h"
#include "../../Utilities/small_helper_structures.h"
#include "../../Utilities/custom_vector.h"
#include "explanation_clausal.h"

#include <vector>

namespace Pumpkin
{

class SolverState; //circular dependency
class ClauseDatabase;

/*The class represents a clause_ with the two-watch scheme that helps detecting binary clause_ propagation.
todo more explanation on what it does
*/

class TwoWatchedClause
{
public:

	void operator delete(void* p) {
		::operator delete(p);// , sizeof(TwoWatchedClause) + literals_.Size() * sizeof(BooleanLiteral));
	}

	WatchedLiterals GetWatchedLiterals() const; //returns the two currently watched learned_clause_literals
	BooleanLiteral GetOtherWatchedLiteral(BooleanLiteral watched_literal) const;

	int Size() const; //returns the number of literals in the clause_
	bool IsBinary() const; //returns if the Size of the clause_ is two
	
	//remove, never used? BooleanLiteral operator[](int i) const; //returns the i-th literal in the clause_ (zero-index based)

	bool updateLBD(const SolverState &state);//updates the lbd score of the clause_ if it is better. Returns true if there was a change. Note: does not update if the LBD is already set to two or less.
	bool IsSatisfied(const SolverState &state);

	static int computeLBD(std::vector<BooleanLiteral> &literals, const SolverState &state); //computes the LBD score for the literals, i.e. the number of unique decision levels in the current clause. See "On the Glucose SAT Solver" by Gilles Audemard and Laurent Simon for info on the LBD.
	static int computeLBD(LiteralVector &literals, const SolverState &state);

	double activity_; //even though activities are used only for learned clauses, they are kept for all clauses
	int best_literal_blocking_distance_;
	bool lbd_update_protection_;
	bool locked_in_propagation_;
	bool is_learned_;
	
	//only clause_ database should create clauses - will forbid others to create todo
	friend ClauseDatabase;

	//only the clause database can create clauses
	//create and initialises a new clause
	//note that the watches will be set according to the state. In case the clause is unsat, then the watches are the literals with the highest decision levels
	static TwoWatchedClause* MakeTwoWatchedClause(const Disjunction &literals, bool is_learned, const SolverState &state); 
	
	//used only when building the initial formula
	//todo I think I should remove this method, better to always call the one above which also properly sets the watches
	static TwoWatchedClause* MakeTwoWatchedClause(const Disjunction &literals); 
	
	//set the first two literals in the p_literals_ as the watches. Watches are primarily unassigned variables, or the ones with the highest decision levels, with [0].decision_level >= [1].decision_level
	//todo: perhaps the watchers should also be the ones furthest down the trail
	void SetWatches(const SolverState&); 

	bool ShouldPropagate(const SolverState&) const; //for debugging purposes
	bool HaxPrint(const SolverState&) const;

	//return if p1 is better than p2 in terms of watching the clause
	static bool WatcherComparison(const BooleanLiteral p1, const BooleanLiteral p2, const SolverState &);

	LiteralVector literals_;
};

} //end Pumpkin namespace

#endif // !TWO_WATCHED_CLAUSE_H