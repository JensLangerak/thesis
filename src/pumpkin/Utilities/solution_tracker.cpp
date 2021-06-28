#include "solution_tracker.h"
#include "../../logger/logger.h"
#include "runtime_assert.h"

#include <iostream>

namespace Pumpkin
{
SolutionTracker::SolutionTracker() :
	upper_bound_(INT64_MAX),
	best_solution_(0),
	is_optimal_(false)
{
}

SolutionTracker::SolutionTracker(const LinearBooleanFunction& objective_function, const Stopwatch& initial_stopwatch):
	stopwatch_(initial_stopwatch),
	upper_bound_(INT64_MAX),
	best_solution_(0),
	is_optimal_(false),
	objective_function_(objective_function)
{
}

bool SolutionTracker::UpdateBestSolution(const BooleanAssignmentVector& solution)
{
	runtime_assert(!is_optimal_);

	int64_t new_upper_bound = objective_function_.ComputeSolutionCost(solution);
	if (new_upper_bound <= upper_bound_)
	{
		if (new_upper_bound < upper_bound_) 
		{ 
			std::cout << "o " << new_upper_bound << "\nc t = " << stopwatch_.TimeElapsedInSeconds() << "\n";
                        simple_sat_solver::logger::Logger::Log2("New upperbound: " + std::to_string(new_upper_bound));
			upper_bound_ = new_upper_bound;
			time_stamps_.AddTimePoint(time_t(stopwatch_.TimeElapsedInSeconds()), new_upper_bound);
		}
		else
		{
			std::cout << "c solution same cost as best: " << upper_bound_ << "\n";			
		}
		best_solution_ = solution;
		return true;
	}
	else
	{
		std::cout << "c failed solution update: " << new_upper_bound << " vs " << upper_bound_ << "\n";
	}
	return false;
}

void SolutionTracker::UpdateOptimalSolution(const BooleanAssignmentVector& solution)
{	
	runtime_assert(!is_optimal_ && objective_function_.ComputeSolutionCost(solution) <= upper_bound_);
	UpdateBestSolution(solution);
}

void SolutionTracker::ExtendOptimalSolution(const BooleanAssignmentVector& solution)
{
	runtime_assert(ComputeCost(solution) == upper_bound_ && best_solution_.NumVariables() <= solution.NumVariables());
	best_solution_ = solution;
}

BooleanAssignmentVector SolutionTracker::GetBestSolution() const
{
	return best_solution_;
}

int64_t SolutionTracker::UpperBound() const
{
	return upper_bound_;
}

bool SolutionTracker::HasFeasibleSolution() const
{
	return upper_bound_ != INT64_MAX;
}

int64_t SolutionTracker::ComputeCost(const BooleanAssignmentVector& solution) const
{
	return objective_function_.ComputeSolutionCost(solution);
}

double SolutionTracker::ComputePrimalIntegral() const
{
	return time_stamps_.ComputePrimalIntegral(stopwatch_.TimeElapsedInSeconds());
}

}
