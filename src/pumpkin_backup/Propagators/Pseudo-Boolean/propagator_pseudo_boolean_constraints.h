#ifndef PROPAGATOR_PSEUDO_BOOLEAN_CONSTRAINTS_H
#define PROPAGATOR_PSEUDO_BOOLEAN_CONSTRAINTS_H

#include "../propagator_generic.h"
#include "watched_pseudo_boolean_constraint.h"
#include "database_pseudo_boolean.h"

namespace Pumpkin
{

class SolverState;

class PropagatorPseudoBooleanConstraints : public PropagatorGeneric
{
public:
	PropagatorPseudoBooleanConstraints(int64_t num_variables);

	ExplanationPseudoBooleanConstraint * ExplainLiteralPropagation(BooleanLiteral literal, SolverState &state); //returns the conjunction that forces the assignment of input literal to true. Assumes the input literal is not undefined.
	ExplanationPseudoBooleanConstraint * ExplainFailure(SolverState &state); //returns the conjunction that leads to failure

	ReasonGeneric * ReasonLiteralPropagation(BooleanLiteral propagated_literal, SolverState &state);
	ReasonGeneric * ReasonFailure(SolverState &state);

	DatabasePseudoBoolean constraint_database_;

	bool PropagateConstraintAtRoot(WatchedPseudoBooleanConstraint *constraint, SolverState &state);

	WatchedPseudoBooleanConstraint *failure_constraint_;
	
private:
	//this is the main propagation method. Note that it will change watch lists of true_literal and some other literals and enqueue assignments
	bool PropagateLiteral2(BooleanLiteral true_literal, SolverState &state);

	bool PropagateLiteral(BooleanLiteral true_literal, SolverState &state); //I think this is not used anymore, should remove it (todo check the differences)

};

} //end Pumpkin namespace

#endif // !PROPAGATOR_PSEUDO_BOOLEAN_CONSTRAINTS_H
