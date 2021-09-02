//
// Created by jens on 10-08-21.
//

#include "extended_groups_adder.h"
#include "../../../Engine/solver_state.h"
namespace Pumpkin {
void PbExtendedGroupsAdder::AddConstraint(PseudoBooleanConstraint &c, SolverState & state) {
  CreateWatchedConstraint(c, state);
}
PbExtendedGroupsModifier *
PbExtendedGroupsAdder::AddConstraintWithModifier(PseudoBooleanConstraint &c,
                                        SolverState &state) {
  WatchedExtendedGroupsConstraint * watched = CreateWatchedConstraint(c, state);
  return new PbExtendedGroupsModifier(watched);
}
WatchedExtendedGroupsConstraint *
PbExtendedGroupsAdder::CreateWatchedConstraint(PseudoBooleanConstraint c,
                                      SolverState &state) {
  if (!propagator_added_)
    AddPropagator(state);

  return propagator_->pseudo_boolean_database_.AddPermanentConstraint(c, state);
}
void PbExtendedGroupsAdder::AddPropagator(SolverState &state) {
  if (!state.HasPropagator(propagator_)) {
    state.AddPropagator(propagator_);
    propagator_added_ = true;
    for (int i = 0; i < state.GetNumberOfVariables(); ++i)
      propagator_->GrowDatabase();
  }
}
bool PbExtendedGroupsModifier::UpdateMax(int max, SolverState &state) {
  watched_constraint_->max_ = max;
  return true;
}
}
