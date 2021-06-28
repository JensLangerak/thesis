//
// Created by jens on 07-06-21.
//

#include "watched_pb_pairs_constraint.h"
#include "explantion_pb_pairs_constraint.h"
#include "../../../Engine/solver_state.h"
namespace Pumpkin {
WatchedPbPairsConstraint::WatchedPbPairsConstraint(
    PseudoBooleanConstraint &constraint)
    : max_(constraint.right_hand_side), current_sum_value_(0),
      decision_level_sums_(std::stack<DecisionLevelCount>()) {
  assert(constraint.literals.size() == constraint.coefficients.size());
  auto literals = constraint.literals;
  auto weights = constraint.coefficients;
  max_weight_ = 0;
  for (int i = 0; i < literals.size(); ++i) {
    intput_liters_.push_back(WeightedLiteral(literals[i], weights[i]));
    lit_weights_[literals[i].ToPositiveInteger()] = weights[i];
    if (weights[i] > max_weight_)
      max_weight_ = weights[i];
    lit_sum_ += weights[i];
  }
  for (BooleanLiteral l : literals) {
    lit_count_[l.ToPositiveInteger()] = 0;
  }
  log_id_ = next_log_id_;
  ++next_log_id_;
}
ExplanationGeneric *WatchedPbPairsConstraint::ExplainLiteralPropagation(
    BooleanLiteral literal, SolverState &state,
    ExplanationDynamicConstraint *explanation) {
  ExplanationPbPairsConstraint::InitExplanationPbPairsConstraint(
      this, state, literal, explanation);
  return explanation;
}
ExplanationGeneric *WatchedPbPairsConstraint::ExplainFailure(
    SolverState &state, ExplanationDynamicConstraint *explanation) {
  ExplanationPbPairsConstraint::InitExplanationPbPairsConstraint(
      this, state, explanation);
  return explanation;
}
void WatchedPbPairsConstraint::AddScheduledEncoding(SolverState &state) {
  pairs_database_->AddScheduled(state);
}

void WatchedPbPairsConstraint::UpdateDecisionLevelSum(
    SolverState &state) {
  if (decision_level_sums_.empty() ||
      decision_level_sums_.top().level < state.GetCurrentDecisionLevel()) {
    decision_level_sums_.push(DecisionLevelCount(
        state.GetCurrentDecisionLevel(), current_sum_value_));
  } else {
    assert(decision_level_sums_.top().level == state.GetCurrentDecisionLevel());
    decision_level_sums_.top().sum_value = current_sum_value_;
  }
}
void WatchedPbPairsConstraint::Synchronise(SolverState &state) {
  while ((!decision_level_sums_.empty()) &&
      decision_level_sums_.top().level > state.GetCurrentDecisionLevel()) {
    decision_level_sums_.pop();
  }
  if (decision_level_sums_.empty())
    current_sum_value_ = 0;
  else
    current_sum_value_ = decision_level_sums_.top().sum_value;
}
bool WatchedPbPairsConstraint::CountCorrect(SolverState &state,
                                                   BooleanLiteral literal) {
  return true;
  std::vector<DecisionLevelCount> stack;
  while (!decision_level_sums_.empty()) {
    DecisionLevelCount countitem = decision_level_sums_.top();
    stack.insert(stack.begin(), countitem);
    decision_level_sums_.pop();
  }
  for (DecisionLevelCount d : stack)
    decision_level_sums_.push(d);

  for (int i = 0; i + 1 < stack.size(); ++i) {
    int level = stack[i].level;
    int sum = stack[i].sum_value;
    int count = 0;

    for (auto wl : intput_liters_) {
      if (state.assignments_.IsAssignedTrue(wl.literal) &&
          state.assignments_.GetAssignmentLevel(wl.literal) <= level) {
        count += wl.weight;
        //        std::cout << state.assignments_.GetAssignmentLevel(wl.literal)
        //        << std::endl;
      }
    }
    assert(count == sum);
  }

  int count = 0;
  for (auto wl : intput_liters_) {
    if (state.assignments_.IsAssignedTrue(wl.literal) &&
        state.assignments_.GetTrailPosition(wl.literal.Variable()) <=
            state.assignments_.GetTrailPosition(literal.Variable()))
      count += wl.weight;
  }
  assert(count == current_sum_value_);
  return count == current_sum_value_;
}
}
