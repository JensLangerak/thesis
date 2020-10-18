#include "propagator_generic.h"
#include "../Engine/solver_state.h"

namespace Pumpkin {

PropagatorGeneric::PropagatorGeneric()
    : next_position_on_trail_to_propagate_(0) {}

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
  next_position_on_trail_to_propagate_ =
      std::min(next_position_on_trail_to_propagate_,
               state.GetNumberOfAssignedVariables());
//  next_position_on_trail_to_propagate_it = state.GetTrailEnd();
//  next_position_on_trail_to_propagate_it--;
}

BooleanLiteral
PropagatorGeneric::GetAndPopNextLiteralToPropagate(SolverState &state) {
  	BooleanLiteral return_literal =
  state.GetLiteralFromTrailAtPosition(next_position_on_trail_to_propagate_);
        next_position_on_trail_to_propagate_++;
//  BooleanLiteral return_literal = *next_position_on_trail_to_propagate_it;
//  next_position_on_trail_to_propagate_it++;
//  if (next_position_on_trail_to_propagate_it == state.GetTrailEnd()) {
//    next_position_on_trail_to_propagate_it--;
//    it_end = true;
//  } else {
//    it_end = false;
//  }
  return return_literal;
}

bool PropagatorGeneric::IsPropagationComplete(SolverState &state) {
  assert(next_position_on_trail_to_propagate_ <=
         state.GetNumberOfAssignedVariables());
//  if (next_position_on_trail_to_propagate_ == 0) {
//    if (state.GetNumberOfAssignedVariables() == 0)
//      return true;
//    next_position_on_trail_to_propagate_it = state.GetTrailBegin();
//    it_end = false;
//  }
//  auto next = next_position_on_trail_to_propagate_it;
//  next++;
//  return next == state.GetTrailEnd() && it_end;
  return next_position_on_trail_to_propagate_ ==
         state.GetNumberOfAssignedVariables();
}

} // namespace Pumpkin