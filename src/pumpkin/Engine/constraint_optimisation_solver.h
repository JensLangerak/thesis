#pragma once

#include "constraint_satisfaction_solver.h"
#include "../Basic Data Structures/problem_specification.h"
#include "../Basic Data Structures/stopwatch.h"
#include "../Basic Data Structures/solver_output.h"

#include <limits>
#include <vector>

namespace Pumpkin {

class GeneralisedTotaliserCP192; //this declaration is not an elegant solution and will be redesigned in the future but it works for now. Needed because of circular dependency with the State class

class ConstraintOptimisationSolver
{
public:
	ConstraintOptimisationSolver(ProblemSpecification& problem_specification, SolverParameters& parameters);
        ~ConstraintOptimisationSolver() ;
	SolverOutput Solve(double time_limit_in_seconds = std::numeric_limits<double>::max());

	void PrintStats();

  ConstraintSatisfactionSolver constrained_satisfaction_solver_;
  IEncoder<PseudoBooleanConstraint>::IFactory * optimisation_encoding_factory;
private:
	//adds a constraint to the constraint_satisfaction_solver forcing the objective to be better than the upper bound. 
	//After the first call, it will create new variables and clauses since the constraint is encoded into CNF
	bool StrengthenUpperBoundConstraints();

	//updates the upper bound, best solution, and records the time stamp. Assumes the input solution is better than the previous solution.
	void UpdateBestSolution(const std::vector<bool>& solution);

	//small helper methods
	bool HasFeasibleSolution() const;
	bool StoppingCriteriaMet() const;
	SolverOutput GenerateOutput() const; //produces a SolutionCostPair based on the best solution recorded
	int64_t ComputeSolutionCost(const std::vector<bool>& solution) const;//computes the objective value of the input solution. Does not check if it is satisfiabile.
	static bool IsLiteralTrue(BooleanLiteral literal, const std::vector<bool>& solution);//helper method to evaluate a literal given a solution

//class variables------------------
//	GeneralisedTotaliserCP192* encoder_; //this encoder needs to be redesigned/improved
  WatchedPseudoBooleanConstraint2 *optimise_constraint = nullptr;
  WatchedPseudoBooleanConstraint2 *optimise_constraint5 = nullptr;
  WatchedPseudoBooleanConstraint2 *optimise_constraint10 = nullptr;
	Stopwatch stopwatch_;
	int64_t lower_bound_, upper_bound_;
	TimeStamps solution_time_stamps_;
	std::vector<bool> best_solution_;
	std::vector<WeightedLiteral> objective_literals_;//represents the linear objective function composing of these weighted literals
	bool activated_;

      public:
        bool UpdateBestSolutionConstraint(int64_t max_cost);
        int64_t start_upper_bound_;
};

}//end Pumpkin namespace