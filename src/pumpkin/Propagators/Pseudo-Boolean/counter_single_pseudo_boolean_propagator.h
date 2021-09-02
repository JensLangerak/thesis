#pragma once

#include "explanation_single_counter_pseudo_boolean_constraint.h"
#include "../propagator_generic.h"
#include "../explanation_generic.h"
#include "../explanation_generator.h"
#include "../reason_generic.h"
#include "../../Utilities/small_helper_structures.h"
#include "../../Utilities/linear_boolean_function.h"
#include "../../Utilities/boolean_assignment_vector.h"

namespace Pumpkin
{
class SolverState;
//represents a constraint of the form \sum w_i x_i <= UB
class PropagatorCounterSinglePseudoBoolean : public PropagatorGeneric
{
public:
	PropagatorCounterSinglePseudoBoolean(bool bump_objective_literals);

	bool Activate(SolverState& state, LinearBooleanFunction &objective_function, int64_t upper_bound); //activate only at root level; initialises the propagator. Returns true if successfully initialised, false indicates that the constraint is false at the root level.
	void Deactivate();
	bool StrengthenUpperBound(SolverState &state, int64_t new_upper_bound); //returns true if successfully, false means tightening the constraint leads to unsat

	void Synchronise(SolverState& state);

	int DebugSum(BooleanAssignmentVector& sol);

	ExplanationGeneric* ExplainLiteralPropagation(BooleanLiteral literal, SolverState& state); //returns the conjunction that forces the assignment of input literal to true. Assumes the input literal is not undefined.
	ExplanationGeneric* ExplainFailure(SolverState& state); //returns the conjunction that leads to failure

	ReasonGeneric* ReasonLiteralPropagation(BooleanLiteral literal, SolverState& state) { runtime_assert(1 == 2); return nullptr; }
	ReasonGeneric* ReasonFailure(SolverState& state) { runtime_assert(1 == 2); return nullptr; }

//private:

	void BumpVariables(int end_index, SolverState& state);
	int64_t MinWeight() const;

	bool PropagateLiteral(BooleanLiteral true_literal, SolverState& state);

	struct Entry { int64_t weight; std::vector<BooleanLiteral> literals; };
	std::vector<Entry> weighted_literals_;
	LinearBooleanFunction objective_function_;
	std::vector<BooleanLiteral> violating_literals;
	int64_t upper_bound_;
	int64_t slack_;
	bool activated_, bump_objective_literals_;
	ExplanationGenerator<ExplanationSingleCounterPseudoBooleanConstraint> explanation_generator_;
};

} //end Pumpkin namespace