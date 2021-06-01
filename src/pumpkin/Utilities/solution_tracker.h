#pragma once

#include "stopwatch.h"
#include "small_helper_structures.h"
#include "linear_boolean_function.h"
#include "boolean_assignment_vector.h"

namespace Pumpkin
{
class SolutionTracker
{
public:
	SolutionTracker();
	SolutionTracker(const LinearBooleanFunction& objective_function, const Stopwatch &initial_stopwatch = Stopwatch());
	
	bool UpdateBestSolution(const BooleanAssignmentVector& solution); //only updates the best solution if it is better than any other solution seen so far. Returns true if the new solution has been accepted.
	void UpdateOptimalSolution(const BooleanAssignmentVector& solution); //should be a vector of BooleanLiteral/BooleanVariable that is referenced by index...or something like that
	void ExtendOptimalSolution(const BooleanAssignmentVector& solution); //only use to add values to additional variables, but the cost should be the case. Used within the upper bounding algorithm to make sure that auxiliary variables are assigned values.

	BooleanAssignmentVector GetBestSolution() const;
	int64_t UpperBound() const;
	bool HasFeasibleSolution() const;

	int64_t ComputeCost(const BooleanAssignmentVector& solution) const;
	double ComputePrimalIntegral() const;


private:
	Stopwatch stopwatch_;
	int64_t upper_bound_;
	BooleanAssignmentVector best_solution_;
	bool is_optimal_;
	TimeStamps time_stamps_;	
	LinearBooleanFunction objective_function_;
};
}