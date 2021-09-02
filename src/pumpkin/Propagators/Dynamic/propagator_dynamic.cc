//
// Created by jens on 21-05-21.
//

#include "propagator_dynamic.h"
#include "../../Engine/solver_state.h"
#include "BottomLayers/watched_bottom_layers_constraint.h"
#include "Pairs/watched_pb_pairs_constraint.h"
#include "PseudoBoolean/watched_pseudo_boolean_constraint_2.h"
#include "Sum/watched_pb_sum_constraint.h"
#include "reason_dynamic_constraint.h"
#include "watched_dynamic_constraint.h"
#include "ExtendedGroups/watched_extended_groups_constraint.h"
namespace Pumpkin {
template <class Watched>
ReasonGeneric *
PropagatorDynamic<Watched>::ReasonLiteralPropagation(BooleanLiteral literal,
                                            SolverState &state) {
  return new ReasonDynamicConstraint(ExplainLiteralPropagation(literal, state));
}
template <class Watched>
ReasonGeneric * PropagatorDynamic<Watched>::ReasonFailure(SolverState &state) {
  return new ReasonDynamicConstraint(ExplainFailure(state));
}
template <class Watched>
ExplanationGeneric *
PropagatorDynamic<Watched>::ExplainLiteralPropagation(BooleanLiteral literal,
                                                      SolverState &state) {
  assert(state.assignments_.GetAssignmentPropagator(
      literal.Variable()) == this);
  uint64_t code =
      state.assignments_.GetAssignmentCode(literal.Variable());
  WatchedDynamicConstraint *propagating_constraint =
      reinterpret_cast<WatchedDynamicConstraint *>(code);
  // TODO check if encoding should be added (if called it is not yet added)
  ExplanationDynamicConstraint* explanation = explanation_generator_.GetAnExplanationInstance();
  return propagating_constraint->ExplainLiteralPropagation(literal, state, explanation);
}
template <class Watched>
ExplanationGeneric *
PropagatorDynamic<Watched>::ExplainFailure(SolverState &state) {
  ExplanationDynamicConstraint* explanation = explanation_generator_.GetAnExplanationInstance();
  return failure_constraint_->ExplainFailure(state, explanation);
}
template <class Watched>
bool PropagatorDynamic<Watched>::PropagateOneLiteral(SolverState &state) {
  if (IsPropagationComplete(state) == false)
  {
    BooleanLiteral propagation_literal = GetNextLiteralToPropagate(state);
    bool success = PropagateLiteral(propagation_literal, state);
    if (success == false)
    {
      return false;
    }
  }
  return true;
}
template <class Watched>
void PropagatorDynamic<Watched>::Synchronise(SolverState &state) {
  PropagatorGeneric::Synchronise(state);
  explanation_generator_.Reset();
  last_propagation_info_ = LastPropagationInfo(0, BooleanLiteral(),0,0);
}
template class PropagatorDynamic<WatchedPseudoBooleanConstraint2>;
template class PropagatorDynamic<WatchedPbPairsConstraint>;
template class PropagatorDynamic<WatchedPbSumConstraint>;
template class PropagatorDynamic<WatchedBottomLayersConstraint>;
template class PropagatorDynamic<WatchedExtendedGroupsConstraint>;
}
