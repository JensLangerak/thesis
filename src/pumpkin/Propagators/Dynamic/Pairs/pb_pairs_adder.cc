//
// Created by jens on 07-06-21.
//

#include "pb_pairs_adder.h"
#include "../../../Engine/solver_state.h"
namespace Pumpkin {
void PbPairsAdder::AddConstraint(PseudoBooleanConstraint &c, SolverState & state) {
  CreateWatchedConstraint(c, state);
}
PbPairsModifier *
PbPairsAdder::AddConstraintWithModifier(PseudoBooleanConstraint &c,
                                              SolverState &state) {
  WatchedPbPairsConstraint * watched = CreateWatchedConstraint(c, state);
  return new PbPairsModifier(watched);
}
WatchedPbPairsConstraint *
PbPairsAdder::CreateWatchedConstraint(PseudoBooleanConstraint c,
                                            SolverState &state) {
  if (!propagator_added_)
    AddPropagator(state);

  return propagator_->pseudo_boolean_database_.AddPermanentConstraint(c, state);
}
void PbPairsAdder::AddPropagator(SolverState &state) {
  if (!state.HasPropagator(propagator_)) {
    state.AddPropagator(propagator_);
    propagator_added_ = true;
    for (int i = 0; i < state.GetNumberOfVariables(); ++i)
      propagator_->GrowDatabase();
  }
}
bool PbPairsModifier::UpdateMax(int max, SolverState &state) {
  watched_constraint_->max_ = max;
  return true;
}
}
