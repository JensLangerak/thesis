#include "propagator_clausal_binary.h"
#include "explanation_clausal_binary.h"
#include "../../Engine/solver_state.h"
#include "../../Utilities/runtime_assert.h"

#include <assert.h>

namespace Pumpkin
{

/*PropagatorClausalBinary::PropagatorClausalBinary(int64_t num_variables) :
	database_(num_variables)
{
}

void PropagatorClausalBinary::Synchronise(SolverState& state)
{
	PropagatorGeneric::Synchronise(state);
	failure_literal1_ = BooleanLiteral();
	failure_literal2_ = BooleanLiteral();
	explanation_generator_.Reset();
}

PropagatorGeneric* PropagatorClausalBinary::AddClause(BooleanLiteral literal1, BooleanLiteral literal2, SolverState& state)
{
	runtime_assert(state.IsPropagationComplete());

	database_.AddClause(literal1, literal2);

	//enqueue and perform propagation
	//note that at most one of the two following IF statements may execute

	if (state.assignments_.IsAssignedFalse(literal1))
	{
		bool enqueue_success = state.EnqueuePropagatedLiteral(literal2, this, (~literal1).ToPositiveInteger());
		if (enqueue_success == false)
		{
			failure_literal1_ = literal1;
			failure_literal2_ = literal2;
			return this;
		}
	}

	if (state.assignments_.IsAssignedFalse(literal2))
	{
		bool enqueue_success = state.EnqueuePropagatedLiteral(literal1, this, (~literal2).ToPositiveInteger());
		if (enqueue_success == false)
		{
			failure_literal1_ = literal1;
			failure_literal2_ = literal2;
			return this;
		}
	}

	return state.PropagateEnqueued();
}

ExplanationGeneric* PropagatorClausalBinary::ExplainLiteralPropagation(BooleanLiteral literal, SolverState& state)
{
	runtime_assert(state.assignments_.GetAssignmentPropagator(literal.Variable()) == this);
	//recover the original propagation. Code holds the integer code of the literal that implied 'literal'
	int code = int(state.assignments_.GetAssignmentCode(literal.Variable()));
	BooleanLiteral implying_literal = BooleanLiteral::IntToLiteral(code);
	//set an explanation for the implying literal
	ExplanationClausalBinary* explanation = explanation_generator_.GetAnExplanationInstance();
	explanation->Initialise(implying_literal);
	return explanation;
}

ExplanationGeneric* PropagatorClausalBinary::ExplainFailure(SolverState& state)
{
	runtime_assert(!failure_literal1_.IsUndefined() && !failure_literal2_.IsUndefined());

	ExplanationClausalBinary* explanation = explanation_generator_.GetAnExplanationInstance();
	explanation->Initialise(~failure_literal1_, ~failure_literal2_); //the literals are negates since the explanation expects a conjuction
	return explanation;
}

ReasonGeneric* PropagatorClausalBinary::ReasonLiteralPropagation(BooleanLiteral literal, SolverState& state)
{
	runtime_assert(1 == 2); //not implemented yet
	return nullptr;
}

ReasonGeneric* PropagatorClausalBinary::ReasonFailure(SolverState& state)
{
	runtime_assert(1 == 2); //not implemented yet
	return nullptr;
}

void PropagatorClausalBinary::Grow()
{
	database_.Grow();
}

bool PropagatorClausalBinary::PropagateLiteral(BooleanLiteral true_literal, SolverState& state)
{
	assert(state.assignments_.IsAssignedTrue(true_literal));

	if (database_.Empty()) { return true; }

	for (BooleanLiteral propagated_literal : database_[~true_literal])
	{
		//try to enqueue the propagated assignment
		int code = true_literal.ToPositiveInteger();
		bool enqueue_success = state.EnqueuePropagatedLiteral(propagated_literal, this, uint64_t(code));
		//it was not possible to even enqueue, conflict occurs
		if (enqueue_success == false)
		{
			failure_literal1_ = ~true_literal;
			failure_literal2_ = propagated_literal;
			return false;
		}
	}
	return true; //no conflicts detected
}*/

}//end namespace Pumpkin
