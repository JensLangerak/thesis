//
// Created by jens on 24-05-21.
//

#include "explanation_pseudo_boolean_constraint_2.h"
#include "../../../Engine/solver_state.h"
#include "../../../Utilities/boolean_variable.h"
#include "../../../Utilities/problem_specification.h"
namespace Pumpkin {
void ExplanationPseudoBooleanConstraint2::
    InitExplanationPseudoBooleanConstraint2(
        WatchedPseudoBooleanConstraint2 *constraint, SolverState &state,
        ExplanationDynamicConstraint *explanation) {
  assert(constraint != nullptr);
  assert(constraint->current_sum_value_ > constraint->max_);
  auto lits_ = std::vector<BooleanLiteral>();
  // Check if the min or max constraint is violated
  bool select_value = constraint->current_sum_value_ > constraint->max_;
  int sum = 0;
  for (auto wl : constraint->current_literals_) {
    BooleanLiteral l = wl.literal;
    if (state.assignments_.IsAssigned(l) &&
        state.assignments_.GetAssignment(l) == select_value) {
      lits_.push_back(l);
      sum += wl.weight;
    }
  }
  assert(sum > constraint->max_);
  explanation->Init(lits_);
  constraint->UpdateCounts(lits_, state);
}
void ExplanationPseudoBooleanConstraint2::
    InitExplanationPseudoBooleanConstraint2(
        WatchedPseudoBooleanConstraint2 *constraint, SolverState &state,
        BooleanLiteral propagated_literal,
        ExplanationDynamicConstraint *explanation) {
  assert(constraint != nullptr);
  assert(state.assignments_.IsAssignedTrue(propagated_literal));
  int l_w = constraint->lit_weights_[(~propagated_literal).ToPositiveInteger()];

  int propagation_level =
      state.assignments_.GetTrailPosition(propagated_literal.Variable());
  auto lits_ = std::vector<BooleanLiteral>();
  int sum = 0;
  for (auto wl : constraint->current_literals_) {
    BooleanLiteral l = wl.literal;
    if (state.assignments_.IsAssigned(l) &&
        state.assignments_.GetTrailPosition(l.Variable()) < propagation_level &&
        state.assignments_.IsAssignedTrue(l)) {
      lits_.push_back(l);
      sum += wl.weight;
    }
  }
  assert(sum + l_w > constraint->max_);
//  lits_.push_back(~propagated_literal); // TODO not sure if correct?
  explanation->Init(lits_);
  constraint->UpdateCounts(lits_, state);
}

} // namespace Pumpkin
