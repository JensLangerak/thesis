//
// Created by jens on 18-06-21.
//

#include "explanation_pb_sum_constraint_output.h"
#include "../../../../Engine/solver_state.h"

namespace Pumpkin {

void ExplanationPbSumConstraintOutput::InitExplanationPbSumOutput(
    WatchedPbSumConstraint *constraint, SolverState &state,
    BooleanLiteral propagated_literal,
    ExplanationDynamicConstraint *explanation) {
  std::vector<BooleanLiteral> lits_;
  BooleanLiteral input = ~propagated_literal;
  assert(constraint->lit_to_input_index_.count(input) > 0);
  int pos = state.assignments_.GetTrailPosition(propagated_literal.Variable());
  WeightedLiteral set_output;
  assert(constraint->outputs_.size() > 0);
  bool found = false;
  for (int i = constraint->set_outputs_.size() - 1; i >= 0; --i) {
    set_output = constraint->set_outputs_[i];
    if (state.assignments_.GetTrailPosition(set_output.literal.Variable()) < pos) {
      found = true;
      break;
    }
  }
  assert(found);
  assert(state.assignments_.IsAssignedFalse(set_output.literal));
  lits_.push_back(~(set_output.literal));
  int index = constraint->lit_to_input_index_[input];
  WeightedLiteral il = constraint->inputs_[index];
  int target_sum = set_output.weight - il.weight;
  for (auto wl : constraint->set_literals_) {
    int i = constraint->lit_to_input_index_[wl];
    WeightedLiteral l = constraint->inputs_[i];
    target_sum -= l.weight;
    lits_.push_back(wl);
    if (target_sum <= 0)
      break;
  }
  assert(target_sum <= 0);

  constraint->UpdateCounts(lits_, state);
  explanation->Init(lits_);
}
void ExplanationPbSumConstraintOutput::InitExplanationPbSumOutput(
    WatchedPbSumConstraint *constraint, SolverState &state,
    ExplanationDynamicConstraint *explanation) {
  assert(!constraint->set_outputs_.empty());
  WeightedLiteral output = constraint->set_outputs_.back();
  auto lits = std::vector<BooleanLiteral>();
  lits.push_back(~output.literal);
  int target_sum = output.weight;
  for (auto l : constraint->set_literals_) {
    int i = constraint->lit_to_input_index_[l];
    WeightedLiteral w = constraint->inputs_[i];
    target_sum -= w.weight;
    lits.push_back(l);
    if (target_sum <= 0)
      break;
  }

  assert(target_sum <= 0);

  constraint->UpdateCounts(lits, state);
  explanation->Init(lits);
}
}