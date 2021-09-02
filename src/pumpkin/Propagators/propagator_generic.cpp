#include "propagator_generic.h"
#include "../Engine/solver_state.h"

namespace Pumpkin
{

PropagatorGeneric::PropagatorGeneric()
	:next_position_on_trail_to_propagate_(0)
{
}

bool PropagatorGeneric::Propagate(SolverState & state)
{
	while (IsPropagationComplete(state) == false)
	{
		BooleanLiteral propagation_literal = GetAndPopNextLiteralToPropagate(state);
		bool success = PropagateLiteral(propagation_literal, state);
		if (success == false)
		{
			return false;
		}
	}
	return true; //no conflicts occurred during propagation
}

bool PropagatorGeneric::PropagateOneLiteral(SolverState & state)
{
	if (IsPropagationComplete(state) == false)
	{
		BooleanLiteral propagation_literal = GetAndPopNextLiteralToPropagate(state);
		bool success = PropagateLiteral(propagation_literal, state);
		if (success == false)
		{
			return false;
		}
	}
	return true; //no conflicts occurred during propagation
}

void PropagatorGeneric::Synchronise(SolverState & state)
{
	next_position_on_trail_to_propagate_ = std::min(next_position_on_trail_to_propagate_, state.GetNumberOfAssignedVariables());
}

BooleanLiteral PropagatorGeneric::GetAndPopNextLiteralToPropagate(SolverState & state)
{
	BooleanLiteral return_literal = state.GetLiteralFromTrailAtPosition(next_position_on_trail_to_propagate_);
	next_position_on_trail_to_propagate_++;
	return return_literal;
}

bool PropagatorGeneric::IsPropagationComplete(SolverState & state)
{
	assert(next_position_on_trail_to_propagate_ <= state.GetNumberOfAssignedVariables());
	return next_position_on_trail_to_propagate_ == state.GetNumberOfAssignedVariables();
}
BooleanLiteral
PropagatorGeneric::GetNextLiteralToPropagate(SolverState &state) {
  BooleanLiteral return_literal = state.GetLiteralFromTrailAtPosition(next_position_on_trail_to_propagate_);
  return return_literal;
}
void PropagatorGeneric::Reset(SolverState &state) {
  next_position_on_trail_to_propagate_ = 0;
}

} //end Pumpkin namespace