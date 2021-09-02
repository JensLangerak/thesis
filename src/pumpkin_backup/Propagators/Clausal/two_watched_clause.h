#ifndef TWO_WATCHED_CLAUSE_H
#define TWO_WATCHED_CLAUSE_H

#include "../../Basic Data Structures/boolean_variable.h"
#include "../../Basic Data Structures/boolean_literal.h"
#include "../../Basic Data Structures/small_helper_structures.h"
#include "../../Basic Data Structures/custom_vector.h"
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

	ExplanationClausal * ExplainLiteralPropagation(BooleanLiteral literal) const; //returns the conjunction that forces the assignment of input literal to true. Assumes the input literal is not undefined.
	ExplanationClausal * ExplainFailure() const; //returns the conjunction that leads to failure
	
	WatchedLiterals GetWatchedLiterals() const; //returns the two currently watched learned_clause_literals
	BooleanLiteral GetOtherWatchedLiteral(BooleanLiteral watched_literal) const;

	int Size() const; //returns the number of literals in the clause_
	bool IsBinary() const; //returns if the Size of the clause_ is two
	
	//remove, never used? BooleanLiteral operator[](int i) const; //returns the i-th literal in the clause_ (zero-index based)

	bool updateLBD(const SolverState &state);//updates the lbd score of the clause_ if it is better. Returns true if there was a change
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

	static TwoWatchedClause* MakeTwoWatchedClause(const Disjunction &literals, bool is_learned, const SolverState &state); //only the clause database can create clauses
	static TwoWatchedClause* MakeTwoWatchedClause(const Disjunction &literals); //used only when building the initial formula
			
	//TwoWatchedClause(const Disjunction &literals, bool is_learned, const SolverState &state); //only the clause database can create clauses
	//TwoWatchedClause(const Disjunction &literals); //used only when building the initial formula
	
									   
												   
	//TwoWatchedClause(const TwoWatchedClause&); //to make clause comparison more efficient when removing clauses, we forbid copying of clauses (we compare memory locations) - maybe this will change

	void SetWatches(const SolverState&); //set the first two literals in the p_literals_ as the watches. Watches are primarily unassigned variables, or the ones with the highest decision levels, with [0].decision_level >= [1].decision_level

	bool ShouldPropagate(const SolverState&) const; //for debugging purposes
	bool HaxPrint(const SolverState&) const;

	//return if p1 is better than p2 in terms of watching the clause
	static bool WatcherComparison(const BooleanLiteral p1, const BooleanLiteral p2, const SolverState &);

	//std::vector<BooleanLiteral> literals_;
	
	LiteralVector literals_;
	//int num_literals_;
	//BooleanLiteral literals_[0];
};

} //end Pumpkin namespace

#endif // !TWO_WATCHED_CLAUSE_H