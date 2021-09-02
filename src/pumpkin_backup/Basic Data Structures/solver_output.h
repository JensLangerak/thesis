#pragma once

#include <vector>

namespace Pumpkin
{

struct SolverOutput
{
	SolverOutput(double runtime_in_seconds, bool timeout_happened, const std::vector<bool>& solution, int64_t cost = 0) :
		runtime_in_seconds(runtime_in_seconds),
		timeout_happened(timeout_happened),
		solution(solution),
		cost(cost)
	{}

	bool HasSolution() const { return !solution.empty(); }
	bool ProvenInfeasible() const { return solution.empty() && !timeout_happened; }

	double runtime_in_seconds;
	bool timeout_happened;
	std::vector<bool> solution;
	int64_t cost;
};

} //end Pumpkin namespace