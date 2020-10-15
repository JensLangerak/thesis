#ifndef PROPAGATOR_CLAUSAL_H
#define PROPAGATOR_CLAUSAL_H

#include "two_watched_clause.h"
#include "clause_database.h"
#include "../propagator_generic.h"
#include "../explanation_generic.h"
#include "../../Basic Data Structures/small_helper_structures.h"

namespace Pumpkin
{

class SolverState;

class PropagatorClausal : public PropagatorGeneric
{
public:
	PropagatorClausal(int64_t num_variables, double decay_factor);

	ExplanationGeneric * ExplainLiteralPropagation(BooleanLiteral literal, SolverState &state); //returns the conjunction that forces the assignment of input literal to true. Assumes the input literal is not undefined.
	ExplanationGeneric * ExplainFailure(SolverState &state); //returns the conjunction that leads to failure

	ReasonGeneric * ReasonLiteralPropagation(BooleanLiteral literal, SolverState &state);
	ReasonGeneric * ReasonFailure(SolverState &state);

	ClauseDatabase clause_database_;

private:
	//this is the main propagation method. Note that it will change watch lists of true_literal and some other literals and enqueue assignments
	bool PropagateLiteral(BooleanLiteral true_literal, SolverState &state);

	void UpdateClauseActivityAndLBD(TwoWatchedClause*, SolverState&);

	TwoWatchedClause *failure_clause_;
};

} //end Pumpkin namespace

#endif // !PROPAGATOR_CLAUSAL_H
