#include "solver_output_checker.h"

namespace Pumpkin
{

bool SolverOutputChecker::CheckSolutionCorrectness(ProblemSpecification& problem_specification, SolverOutput& output, bool verbose)
{
	if (output.HasSolution())
	{
		if (problem_specification.IsSatisfyingAssignment(output.solution) == false)
		{
			if (verbose) std::cout << "Solution not OK: does not satisfy hard constraints!\n";
			return false;
		}

		int64_t computed_cost = problem_specification.ComputeCost(output.solution);
		if (computed_cost != output.cost)
		{
			if (verbose)
			{
				std::cout << "Solution not OK: does not match the reported cost!\n";
				std::cout << "\tReported cost: " << output.cost << "\n";
				std::cout << "\tRecomputed cost: " << problem_specification.ComputeCost(output.solution) << "\n";
			}
			return false;
		}
		else
		{
			std::cout << "c final solution cost: " << computed_cost << "\n";
		}
		if (verbose) std::cout << "c basic solution checks passed!\n";
	}

	/*std::cout << "Num objective literals: " << problem_specification.objective_literals_.size() << "\n";
	std::cout << "Core size: " << output.unsatisfiable_core.size() << " (core weight = ";
	int64_t max_weight = 9999999999999;
	for (auto literal : output.unsatisfiable_core)
	{
		max_weight = std::min(max_weight, problem_specification.GetLiteralWeight(literal));
	}
	std::cout << max_weight << ")\n";*/
	return true;
}
}