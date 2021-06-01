#pragma once

#include "constraint_satisfaction_solver.h"
#include "../Pseudo-Boolean Encoders/encoder_totaliser.h"
#include "../Pseudo-Boolean Encoders/encoder_cardinality_network.h"
#include "../Utilities/linear_boolean_function.h"
#include "../Utilities/solution_tracker.h"
#include "../Utilities/parameter_handler.h"

namespace Pumpkin
{
class LowerBoundSearch
{
public:

	LowerBoundSearch(ParameterHandler& parameters);

	bool Solve
	(
		ConstraintSatisfactionSolver& solver,
		LinearBooleanFunction& objective_function,
		SolutionTracker& solution_tracker,
		int64_t lower_bound_previous_iterations_original_objective,
		double time_limit_in_seconds
	);

private:
	
	int64_t GetInitialWeightThreshold(const LinearBooleanFunction& objective_function);
	int64_t GetNextWeightThreshold(int64_t current_weight_threshold, const LinearBooleanFunction& objective_function);
	int64_t GetNextWeightRatioStrategy(int64_t previous_weight_threshold, const LinearBooleanFunction& objective_function);

	bool HardenAndFilterHardenedAssumptions(std::vector<BooleanLiteral>& assumptions, ConstraintSatisfactionSolver& solver, LinearBooleanFunction& objective_function, SolutionTracker& solution_tracker);

	void CoreGuidedSearchWithWeightThreshold(
		int64_t weight_threshold,
		Stopwatch& stopwatch,
		ConstraintSatisfactionSolver& constrained_satisfaction_solver_,
		LinearBooleanFunction& objective_function,
		SolutionTracker& solution_tracker,
		int64_t lower_bound_previous_iterations_original_objective
	);

	std::vector<BooleanLiteral> InitialiseAssumptions(int64_t weight_threshold, const LinearBooleanFunction& objective_function);
	int64_t GetMinimumCoreWeight(Disjunction& core_clause, const LinearBooleanFunction& objective_function);
	void DecreaseCoreLiteralWeights(Disjunction& core_clause, LinearBooleanFunction& objective_function);
	void ProcessCores(std::vector<Disjunction>& core_clauses, std::vector<int64_t> core_weights, ConstraintSatisfactionSolver& solver, LinearBooleanFunction& objective_function);
	void FilterAssumptionsBasedOnWeightThreshold(std::vector<BooleanLiteral> &assumptions, const Disjunction& core_clause, int64_t weight_threshold, const LinearBooleanFunction &objective_function);
	//parameters
	enum class CardinalityConstraintEncoding { TOTALISER, CARDINALITY_NETWORK } cardinality_constraint_encoding_; //todo consider abstract classes instead of this
	enum class StratificationStrategy {OFF, BASIC, RATIO} stratification_strategy_;
	bool use_weight_aware_core_extraction_;
	//other variables
	EncoderTotaliser totaliser_encoder_;
	EncoderCardinalityNetwork cardinality_network_encoder_;
};
}