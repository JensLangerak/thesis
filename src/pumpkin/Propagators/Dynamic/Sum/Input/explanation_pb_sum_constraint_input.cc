//
// Created by jens on 18-06-21.
//

#include "explanation_pb_sum_constraint_input.h"
#include "../../../../Engine/solver_state.h"

namespace Pumpkin {

void ExplanationPbSumConstraintInput::InitExplanationPbSumInput(
    WatchedPbSumConstraint *constraint, SolverState &state,
    ExplanationDynamicConstraint *explanation) {
  assert(constraint != nullptr);
  auto lits_ = std::vector<BooleanLiteral>();
  for (BooleanLiteral l : constraint->set_literals_) {
    lits_.push_back(l);
  }
  if (constraint->true_count_ <= constraint->max_) {
    assert(!constraint->set_outputs_.empty());
    lits_.push_back(constraint->set_outputs_.back().literal);
  }
  constraint->UpdateCounts(lits_, state);
  explanation->Init(lits_);
}
void ExplanationPbSumConstraintInput::InitExplanationPbSumInput(
    WatchedPbSumConstraint *constraint, SolverState &state,
    BooleanLiteral propagated_literal,
    ExplanationDynamicConstraint *explanation) {
  int lit_index = constraint->lit_to_output_index_[propagated_literal];
  WeightedLiteral output = constraint->outputs_[lit_index];
  assert(state.assignments_.IsAssignedTrue(output.literal));
  std::vector<BooleanLiteral> lits_;
  int sum = 0;
  for (BooleanLiteral l : constraint->set_literals_) {
    assert(state.assignments_.GetTrailPosition(l.Variable()) < state.assignments_.GetTrailPosition(propagated_literal.Variable()));
    lits_.push_back(l);
    int index = constraint->lit_to_input_index_[l];
    sum += constraint->inputs_[index].weight;
    if (sum >= output.weight)
      break;
  }
  assert(sum >= output.weight);
  constraint->UpdateCounts(lits_, state);
  explanation->Init(lits_);
}
}