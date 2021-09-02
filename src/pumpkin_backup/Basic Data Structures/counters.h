#ifndef COUNTERS_H
#define COUNTERS_H

#include "solver_parameters.h"

namespace Pumpkin
{

class Counters
{
public:

	long long conflicts; //counts the number of conflicts occurred during search
	long long restarts;
	long long decisions;


	int unit_clauses_learned;
	int small_lbd_clauses_learned;
	int ternary_clauses_learned;
	
	int clause_cleanup;
	int until_clause_cleanup;

	int blocked_restarts;

	int conflicts_until_restart; //denotes the number of conflict until the solver considers restarting

	void PrintStats() const;

	Counters(SolverParameters &parameters) 
		:conflicts(0),
		restarts(0),
		decisions(0),
		unit_clauses_learned(0),
		small_lbd_clauses_learned(0),	
		ternary_clauses_learned(0),
		clause_cleanup(0),
		until_clause_cleanup(parameters.num_min_conflicts_per_clause_cleanup),
		blocked_restarts(0),
		conflicts_until_restart(parameters.num_min_conflicts_per_restart)
	{
	}
};

} //end Pumpkin namespace

#endif // !COUNTERS_H