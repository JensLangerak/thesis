#ifndef CLAUSE_DATABASE_H
#define CLAUSE_DATABASE_H

class BooleanLiteral;
class TwoWatchedClause;
class SolverState;

#include "watch_list.h"
#include "../../Basic Data Structures/exponential_moving_average.h"

#include <vector>

namespace Pumpkin
{

class ClauseDatabase
{
public:
	ClauseDatabase(int64_t num_variables, double decay_factor);

	//---Methods to add clauses to the database

	TwoWatchedClause * AddPermanentClause(std::vector<BooleanLiteral>&, const SolverState&);//creates a clause_ based on the input literals (which are seen as a disjunction), adds it to the database permanently, and returns a pointer to the newly created clause_. Permanent clauses are never deleted, hence as long as this class is alive the pointer should be valid
	TwoWatchedClause * AddTemporaryClause(std::vector<BooleanLiteral>&, const SolverState&);//creates a clause_ based on the input literals (which are seen as a disjunction), adds it as a learned/temporary clause_ to the database, and returns a pointer to the newly created class. Note that learned/temporary clauses might later be removed by 'ReduceLearnedClauses' and therefore it is essential to keep this in mind when operating with the returned pointer
	
	//The unit clauses need to be added before the search starts. 
	//at the moment, for now I need to be careful when adding these unit clauses during the search. This is done in a special way during search, e.g. backtracking to level 0 and then enqueuing the literal.
	//could consider adding a version which takes the state as input
	void AddUnitClause(BooleanLiteral&);
	void AddUnitClauses(std::vector<BooleanLiteral> &units);

	void AddBinaryClause(BooleanLiteral, BooleanLiteral, const SolverState&);
	void AddTernaryClause(BooleanLiteral, BooleanLiteral, BooleanLiteral, const SolverState&);

	//--- Other methods below...

	void ReduceTemporaryClauses(); //removes half of the least active learned clauses from the database. For now this should only be done at the root level during the search since there is not record on whether a clause_ is involved in a propagation (todo).

	void BumpClauseActivity(TwoWatchedClause*);  //bumps the activity of the clause_ -> used in conflict analysis, so that in ReduceTemporaryClauses() clauses with high activity are prefered over those with low activity 
	void DecayClauseActivities(); //decay the activity of all clauses
	
	int NumberOfLearnedClauses() const; //returns the number of learned clauses currently located in the database. TODO might not be realistic since some might be permanently stored
	int NumberOfClausesTotal() const; //this will be tricky to maintain with new improvements that implicitly store clauses, TODO

	//a bit hacky but okay for now
	std::vector<TwoWatchedClause*> permanent_clauses_;

	//debug methods
	bool NothingToPropagate(const SolverState&) const; //for debugging purposes
	bool AreWatchesCorrect(const SolverState&) const;

	//should be private...
	void AddWatchers(TwoWatchedClause * clause); //can these be const clauses?
	void RemoveWatchers(TwoWatchedClause * clause);

	WatchList watch_list_;
	std::vector<BooleanLiteral> unit_clauses_;
	long long counter_total_removed_clauses_;
	long long number_of_learned_literals_;
	long long number_of_learned_clauses_;

	TwoWatchedClause * GetPermanentClause(std::vector<BooleanLiteral> &literals);

	bool AreClausesProperlyWatched(const std::vector<TwoWatchedClause*>&, const SolverState&) const;
	bool IsClauseProperlyWatched(const TwoWatchedClause * const, const SolverState&) const;

	void RecomputeAndPrintClauseLengthStatsForPermanentClauses();

private:
	void RescaleClauseActivities(); //divides all activities with a large number when the maximum activity becomes too large
	
	static bool ClauseComparisonCriteria(const TwoWatchedClause * c1, const TwoWatchedClause * c2);

	bool IsLiteralProperlyWatched(BooleanLiteral, const SolverState&) const;

	std::vector<TwoWatchedClause*> temporary_clauses_;
	double increment_, max_threshold_, decay_factor_;	
};

} //end Pumpkin namespace

#endif // !CLAUSE_DATABASE_H