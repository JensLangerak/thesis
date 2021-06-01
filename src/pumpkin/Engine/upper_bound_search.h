#pragma once

#include "../Propagators/Pseudo-Boolean/counter_single_pseudo_boolean_propagator.h"
#include "../Propagators/i_constraint_adder.h"
#include "../Pseudo-Boolean Encoders/encoder_generalised_totaliser.h"
#include "../Utilities/linear_boolean_function.h"
#include "../Utilities/parameter_handler.h"
#include "../Utilities/solution_tracker.h"
#include "constraint_satisfaction_solver.h"

namespace Pumpkin
{
class UpperBoundSearch
{
public:
	UpperBoundSearch(ParameterHandler& parameters);
        ~UpperBoundSearch();

	bool Solve
	(
		ConstraintSatisfactionSolver& solver,
		LinearBooleanFunction& objective_function,
		SolutionTracker& solution_tracker,
		int64_t lower_bound_previous_iterations_original_objective,
		double time_limit_in_seconds
	);

private:
	void LinearSearch
	(
		ConstraintSatisfactionSolver& solver,
		LinearBooleanFunction& objective_function,
		SolutionTracker& solution_tracker,
		int64_t lower_bound_original_objective, //the lower bound is with respect to the original objective used in the solution tracker, todo maybe merge with solution tracker
		double time_limit_in_seconds
	);

	//encodes a pseudo-Boolean constraint is encodes that the assignment must be less or equal to the upper bound
	//todo think about moving everything related to linear constraints into the encoder class rather than keep it here
	std::vector<PairWeightLiteral> EncodeInitialUpperBound(ConstraintSatisfactionSolver& solver, LinearBooleanFunction& objective_function, int64_t upper_bound, double time_limit_in_seconds);
	bool StrengthenUpperBound(const std::vector<PairWeightLiteral>& sum_literals, int64_t upper_bound, ConstraintSatisfactionSolver& solver);
	void SetValueSelectorValues(ConstraintSatisfactionSolver& solver, LinearBooleanFunction& objective_function, const BooleanAssignmentVector& solution);

	BooleanAssignmentVector ComputeExtendedSolution(const BooleanAssignmentVector& reference_solution, ConstraintSatisfactionSolver& solver, double time_limit_in_seconds); //extends the input solution to assign values to the auxiliary variables that are not present in the solution but are part of the solver

	int64_t GetInitialDivisionCoefficient(LinearBooleanFunction& objective_function);
	int64_t GetNextDivisionCoefficient(int64_t current_division_coefficient, const LinearBooleanFunction& objective_function);
	LinearBooleanFunction GetVaryingResolutionObjective(int64_t division_coefficient, LinearBooleanFunction& original_objective);
	int64_t GetNextDivisionCoefficientRatioStrategy(int64_t division_coefficient, const LinearBooleanFunction& objective_function);

	enum class VaryingResolutionStrategy { OFF, BASIC, RATIO } varying_resolution_strategy_;
	enum class ValueSelectionStrategy { PHASE_SAVING, SOLUTION_GUIDED_SEARCH, OPTIMISTIC, OPTIMISTIC_AUX } value_selection_strategy_;
	EncoderGeneralisedTotaliser pseudo_boolean_encoder_;
//	PropagatorCounterSinglePseudoBoolean *upper_bound_propagator_; //should not be deleted by the class if passed to SolverState
        IConstraintAdder<PseudoBooleanConstraint> * upper_bound_adder_ = nullptr;
        IPseudoBooleanConstraintModifier * upper_bound_constraint_ = nullptr;
	std::vector<BooleanLiteral> helper_;
};
}