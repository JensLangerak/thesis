//
// Created by jens on 07-07-21.
//

#include "bottom_layers_adder.h"
#include "../../../Engine/solver_state.h"

namespace Pumpkin {

bool BottomLayersModifier::UpdateMax(int max, SolverState &state) {
  watched_constraint_->max_ = max;
  return true;
}
void BottomLayersAdder::AddPropagator(SolverState &state) {
  if (!state.HasPropagator(propagator_)) {
    state.AddPropagator(propagator_);
    propagator_added_ = true;
    for (int i = 0; i < state.GetNumberOfVariables(); ++i)
      propagator_->GrowDatabase();
  }
}
void BottomLayersAdder::AddConstraint(PseudoBooleanConstraint &c,
                                      SolverState &state) {
  CreateWatchedConstraint(c, state);
}
BottomLayersModifier *
BottomLayersAdder::AddConstraintWithModifier(PseudoBooleanConstraint &c,
                                             SolverState &state) {
  WatchedBottomLayersConstraint * w = CreateWatchedConstraint(c, state);
  return new BottomLayersModifier(w);
}
WatchedBottomLayersConstraint *
BottomLayersAdder::CreateWatchedConstraint(PseudoBooleanConstraint constraint,
                                           SolverState &state) {
  if (!propagator_added_)
    AddPropagator(state);
  return propagator_->database_bottom_layers_.AddPermanentConstraint(constraint, state);
}
}