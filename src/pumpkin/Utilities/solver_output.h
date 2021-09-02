#pragma once

#include "small_helper_structures.h"
#include "boolean_assignment_vector.h"

#include <vector>

namespace Pumpkin
{

struct SolverOutput
{
	SolverOutput(double runtime_in_seconds, bool timeout_happened, const BooleanAssignmentVector& solution, int64_t cost, const Disjunction& core) :
		runtime_in_seconds(runtime_in_seconds),
		timeout_happened(timeout_happened),
		solution(solution),
		cost(cost),
		core_clause(core)
	{}
        SolverOutput() :
            runtime_in_seconds(-1),
            timeout_happened(false),
            solution(),
            cost(-1),
            core_clause()
        {}

	bool HasSolution() const { return !solution.IsEmpty(); }
	bool ProvenInfeasible() const { return solution.IsEmpty() && !timeout_happened; }

	double runtime_in_seconds;
	bool timeout_happened;
	BooleanAssignmentVector solution;
	int64_t cost;
	Disjunction core_clause;
};

} //end Pumpkin namespace