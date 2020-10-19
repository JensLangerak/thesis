#include "propagator_generic.h"
#include "../Basic Data Structures/trail_list.h"
#include "../Engine/solver_state.h"

namespace Pumpkin {

PropagatorGeneric::PropagatorGeneric()
    : next_position_on_trail_to_propagate_(0),
      next_position_on_trail_to_propagate_it(nullptr){}

bool PropagatorGeneric::Propagate(SolverState &state) {
  while (IsPropagationComplete(state) == false) {
    BooleanLiteral propagation_literal = GetAndPopNextLiteralToPropagate(state);
    bool success = PropagateLiteral(propagation_literal, state);
    if (success == false) {
      return false;
    }
  }
  return true; // no conflicts occurred during propagation
}

bool PropagatorGeneric::PropagateOneLiteral(SolverState &state) {
  if (IsPropagationComplete(state) == false) {
    BooleanLiteral propagation_literal = GetAndPopNextLiteralToPropagate(state);
    bool success = PropagateLiteral(propagation_literal, state);
    if (success == false) {
      return false;
    }
  }
  return true; // no conflicts occurred during propagation
}

void PropagatorGeneric::Synchronise(SolverState &state) {
  // TODO
  if (next_position_on_trail_to_propagate_ > state.GetNumberOfAssignedVariables()) {
    while (next_position_on_trail_to_propagate_it != state.GetTrailEnd())
      next_position_on_trail_to_propagate_it.Previous();
  }
  next_position_on_trail_to_propagate_ =
      std::min(next_position_on_trail_to_propagate_,
               state.GetNumberOfAssignedVariables());
  assert(next_position_on_trail_to_propagate_ == state.GetNumberOfAssignedVariables() && next_position_on_trail_to_propagate_it == state.GetTrailEnd()
         || state.GetLiteralFromTrailAtPosition(next_position_on_trail_to_propagate_) == *next_position_on_trail_to_propagate_it);
}

BooleanLiteral
PropagatorGeneric::GetAndPopNextLiteralToPropagate(SolverState &state) {
  	BooleanLiteral return_literal =
  state.GetLiteralFromTrailAtPosition(next_position_on_trail_to_propagate_);
        next_position_on_trail_to_propagate_++;
  BooleanLiteral return_literal2 = *next_position_on_trail_to_propagate_it;
  assert(return_literal == return_literal2);
  next_position_on_trail_to_propagate_it.Next();
  return return_literal;
}

bool PropagatorGeneric::IsPropagationComplete(SolverState &state) {
  assert(next_position_on_trail_to_propagate_ <=
         state.GetNumberOfAssignedVariables());
  bool res1 = next_position_on_trail_to_propagate_it.IsLast();
  bool res2 =  next_position_on_trail_to_propagate_ ==
         state.GetNumberOfAssignedVariables();
  assert(res1 == res2);
  return res2;
}

} // namespace Pumpkin