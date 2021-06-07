//
// Created by jens on 30-05-21.
//

#include "pseudo_boolean_adder.h"
#include "../Encoders/propagator_encoder.h"
#include "../../../Engine/solver_state.h"
namespace Pumpkin {
void PseudoBooleanAdder::AddConstraint(PseudoBooleanConstraint &c, SolverState & state) {
  CreateWatchedConstraint(c, state);
}
PseudoBooleanModifier *
PseudoBooleanAdder::AddConstraintWithModifier(PseudoBooleanConstraint &c,
                                              SolverState &state) {
  WatchedPseudoBooleanConstraint2 * watched = CreateWatchedConstraint(c, state);
  return new PseudoBooleanModifier(watched);
}
WatchedPseudoBooleanConstraint2 *
PseudoBooleanAdder::CreateWatchedConstraint(PseudoBooleanConstraint c,
                                            SolverState &state) {
  if (!propagator_added_)
    AddPropagator(state);

    if (encoder_factory == nullptr)
    encoder_factory =
        new PropagatorEncoder<PseudoBooleanConstraint>::Factory();

  return propagator_->pseudo_boolean_database_.AddPermanentConstraint(c, encoder_factory, state);
}
void PseudoBooleanAdder::AddPropagator(SolverState &state) {
  if (!state.HasPropagator(propagator_)) {
    state.AddPropagator(propagator_);
    propagator_added_ = true;
    for (int i = 0; i < state.GetNumberOfVariables(); ++i)
      propagator_->GrowDatabase();
  }
}
bool PseudoBooleanModifier::UpdateMax(int max, SolverState &state) {
  watched_constraint_->max_ = max;
  return watched_constraint_->encoder_->UpdateMax(max, state);
}
template class IConstraintAdder<PseudoBooleanConstraint>;
}
