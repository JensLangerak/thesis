#pragma once

#include "parameter_handler.h"

#include <stdint.h>

namespace Pumpkin
{
class Counters
{
public:

	int64_t conflicts; //counts the number of conflicts occurred during search
	int64_t num_restarts;
	int64_t decisions;
	int64_t propagations;	

	int unit_clauses_learned;
	int small_lbd_clauses_learned;
	int ternary_clauses_learned;
	
	int num_clause_cleanup;

	int blocked_restarts;

	int conflicts_until_restart; //denotes the number of conflict until the solver considers restarting

	void PrintStats() const;

	Counters(ParameterHandler &parameters):
		conflicts(0),
		num_restarts(0),
		decisions(0),
		propagations(0),
		unit_clauses_learned(0),
		small_lbd_clauses_learned(0),	
		ternary_clauses_learned(0),
		num_clause_cleanup(0),
		blocked_restarts(0),
		conflicts_until_restart(parameters.GetIntegerParameter("num-min-conflicts-per-restart"))
	{}
};
} //end Pumpkin namespace