#pragma once

class BooleanLiteral;
class TwoWatchedClause;
class SolverState;

#include "watch_list.h"
#include "../../Utilities/exponential_moving_average.h"

#include <vector>

namespace Pumpkin
{

class ClauseDatabase
{
public:
	ClauseDatabase
	(
		int64_t num_variables, 
		double decay_factor, 
		int lbd_threshold, 
		int target_max_num_temporary_clauses,
		bool use_LBD_for_sorting_temporary_clauses
	);
        ~ClauseDatabase();

	//---Methods to add clauses to the database

	TwoWatchedClause * AddPermanentClause(std::vector<BooleanLiteral>&, const SolverState&);//creates a clause_ based on the input literals (which are seen as a disjunction), adds it to the database permanently, and returns a pointer to the newly created clause_. Permanent clauses are never deleted, hence as long as this class is alive the pointer should be valid
	TwoWatchedClause * AddLearnedClause(std::vector<BooleanLiteral>&, const SolverState&);//creates a clause_ based on the input literals (which are seen as a disjunction), adds it as a learned/temporary clause_ to the database, and returns a pointer to the newly created class. Note that learned/temporary clauses might later be removed by 'ReduceLearnedClauses' and therefore it is essential to keep this in mind when operating with the returned pointer
	
	//The unit clauses need to be added before the search starts. 
	//at the moment, for now I need to be careful when adding these unit clauses during the search. This is done in a special way during search, e.g. backtracking to level 0 and then enqueuing the literal.
	//could consider adding a version which takes the state as input
	void AddUnitClause(BooleanLiteral&);
	void AddUnitClauses(std::vector<BooleanLiteral> &units);

	void AddBinaryClause(BooleanLiteral, BooleanLiteral, const SolverState&);
	void AddTernaryClause(BooleanLiteral, BooleanLiteral, BooleanLiteral, const SolverState&);

	//--- Other methods below...

	//Temporary clauses that achieved LBD less or equal to the threshold are promoted to the low_lbd_clauses_ category
	//Then half of the least active learned clauses are removed from the clause database.
	// 	   However clauses that improved their LBD since last clean up are protected for one round and not removed
	//For now this should only be done at the root level during the search since there is not record on whether a clause is involved in a propagation.
	void PromoteAndReduceTemporaryClauses();
	
	void RemoveAllLearnedClauses();

	void BumpClauseActivity(TwoWatchedClause*);  //bumps the activity of the clause_ -> used in conflict analysis, so that in ReduceTemporaryClauses() clauses with high activity are prefered over those with low activity 
	void DecayClauseActivities(); //decay the activity of all clauses
	
	int NumLearnedClauses() const; //returns the number of learned clauses currently located in the database. This includes low-lbd clauses and temporary clauses.
	int NumClausesTotal() const; //this will be tricky to maintain with new improvements that implicitly store clauses, TODO
	int NumTemporaryClauses() const;
	bool TemporaryClausesExceedLimit() const;

	//a bit hacky to keep as public but okay for now
	std::vector<TwoWatchedClause*> permanent_clauses_;
	struct LearnedClauses { std::vector<TwoWatchedClause*> low_lbd_clauses, temporary_clauses; } learned_clauses_;

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

	//finds the permanent clause that corresponds to the input literals. Returns NULL if no such clause exists.
	TwoWatchedClause * GetPermanentClause(std::vector<BooleanLiteral> &literals);

	bool AreClausesProperlyWatched(const std::vector<TwoWatchedClause*>&, const SolverState&) const;
	bool IsClauseProperlyWatched(const TwoWatchedClause *, const SolverState&) const;

	void RecomputeAndPrintClauseLengthStatsForPermanentClauses(bool print_clauses = false);

private:
	void RescaleClauseActivities(); //divides all activities with a large number when the maximum activity becomes too large
	
	static bool ClauseComparisonCriteria(const TwoWatchedClause * c1, const TwoWatchedClause * c2);

	bool IsLiteralProperlyWatched(BooleanLiteral, const SolverState&) const;
		
	double increment_, max_threshold_, decay_factor_;	
	int LBD_threshold_;
	int target_max_num_temporary_clauses_; //note that this target may be breached since we only do database clean ups at the root. This number is used to determine the number of clauses to remove when cleaning up the database.
	bool use_LBD_for_sorting_temporary_clauses_;
};

} //end Pumpkin namespace