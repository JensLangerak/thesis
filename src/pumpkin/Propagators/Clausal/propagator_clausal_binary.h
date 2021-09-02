#pragma once

#include "database_clausal_binary.h"
#include "explanation_clausal_binary.h"
#include "../propagator_generic.h"
#include "../explanation_generator.h"

namespace Pumpkin
{
/*class PropagatorClausalBinary : public PropagatorGeneric
{
public:
	PropagatorClausalBinary(int64_t num_variables);

	void Synchronise(SolverState& state);

	//assumes that before calling this method, the solver state has completed propagation, i.e., there are no literals waiting in the propagation queue
	//adds a binary clause to the database and performs propagation 
	//returns the conflicting propagator if a conflict occured, otherwise returns NULL if no conflicts happened
	//if a conflicted took place, conflict analysis should be performed using the propagator to restore the solver to a non-conflicting state or report unsatisfiability
	PropagatorGeneric* AddClause(BooleanLiteral literal1, BooleanLiteral literal2, SolverState &state); 

	ExplanationGeneric* ExplainLiteralPropagation(BooleanLiteral literal, SolverState& state);
	ExplanationGeneric* ExplainFailure(SolverState& state);

	ReasonGeneric* ReasonLiteralPropagation(BooleanLiteral literal, SolverState& state);
	ReasonGeneric* ReasonFailure(SolverState& state);

	void Grow();

private:
	bool PropagateLiteral(BooleanLiteral true_literal, SolverState& state);

	BooleanLiteral failure_literal1_, failure_literal2_; //these two literals compose the failure clause, i.e., a (binary) clause that lead to failure during propagation
	DatabaseClausalBinary database_;
	ExplanationGenerator<ExplanationClausalBinary> explanation_generator_;
};*/
} //end Pumpkin namespace