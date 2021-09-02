//
// Created by jens on 10-08-21.
//

#include "watched_extended_groups_constraint.h"
#include "explanation_extended_groups.h"
#include "../../../Engine/solver_state.h"
namespace Pumpkin {
WatchedExtendedGroupsConstraint::WatchedExtendedGroupsConstraint(
    PseudoBooleanConstraint &constraint)
    : max_(constraint.right_hand_side), current_sum_value_(0),
      decision_level_sums_(std::stack<DecisionLevelCount>()) {
  assert(constraint.literals.size() == constraint.coefficients.size());
  auto literals = constraint.literals;
  auto weights = constraint.coefficients;
  max_weight_ = 0;
  for (int i = 0; i < literals.size(); ++i) {
    intput_liters_.push_back(WeightedLiteral(literals[i], weights[i]));
    lit_weights_[literals[i]] = weights[i];
    if (weights[i] > max_weight_)
      max_weight_ = weights[i];
    lit_sum_ += weights[i];
  }
  for (BooleanLiteral l : literals) {
    lit_count_[l] = 0;
  }
  log_id_ = next_log_id_;
  ++next_log_id_;
}
ExplanationGeneric *WatchedExtendedGroupsConstraint::ExplainLiteralPropagation(
    BooleanLiteral literal, SolverState &state,
    ExplanationDynamicConstraint *explanation) {
  ExplanationExtendedGroups::InitExplanationExtendedGroups(
      this, state, literal, explanation);
  add_aux_literal_ = true;
  return explanation;
}
ExplanationGeneric *WatchedExtendedGroupsConstraint::ExplainFailure(
    SolverState &state, ExplanationDynamicConstraint *explanation) {
  ExplanationExtendedGroups::InitExplanationExtendedGroups(
      this, state, explanation);
  add_aux_literal_ = true;
  return explanation;
}
void WatchedExtendedGroupsConstraint::AddScheduledEncoding(SolverState &state) {
//  pairs_database_->AddScheduled(state);
}

void WatchedExtendedGroupsConstraint::UpdateDecisionLevelSum(
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
void WatchedExtendedGroupsConstraint::Synchronise(SolverState &state) {
  while ((!decision_level_sums_.empty()) &&
      decision_level_sums_.top().level > state.GetCurrentDecisionLevel()) {
    decision_level_sums_.pop();
  }
  if (decision_level_sums_.empty())
    current_sum_value_ = 0;
  else
    current_sum_value_ = decision_level_sums_.top().sum_value;

  while((!set_literals_.empty())&&(!state.assignments_.IsAssignedTrue(set_literals_.back()))) {
    set_literals_.pop_back();
    if ((!auxiliray_boundary_.empty()) && auxiliray_boundary_.back() >= set_literals_.size()) {
      auxiliary_literals_.pop_back();
      auxiliray_boundary_.pop_back();
    }
  }
  for (BooleanLiteral l : set_literals_) {
    assert(state.assignments_.IsAssignedTrue(l));
  }

  if (add_aux_literal_ && (!set_literals_.empty())) {
    if (auxiliary_literals_.empty() || auxiliray_boundary_.back() < set_literals_.size()) {
      if (state.assignments_.GetAssignmentLevel(set_literals_.back()) == state.GetCurrentDecisionLevel()) {
        AddAuxiliaryLiteral(state);
      }
    }
  }
  add_aux_literal_ = false;


}
bool WatchedExtendedGroupsConstraint::CountCorrect(SolverState &state,
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
void WatchedExtendedGroupsConstraint::AddAuxiliaryLiteral(SolverState &state) {
  std::vector<BooleanLiteral> clause;
  BooleanLiteral sumLit = BooleanLiteral(state.CreateNewVariable(), true);
  clause.push_back(sumLit);
  int start = 0;
  if (!auxiliary_literals_.empty()) {
    clause.push_back(~auxiliary_literals_.back());
    start = auxiliray_boundary_.back();
    assert(state.assignments_.IsAssignedTrue(auxiliary_literals_.back()));
  }
#ifdef USEFAILURE
  bool new_false_lit = false_literal_.IsUndefined() || state.assignments_.IsAssignedFalse(false_literal_) || create_new_failure_;
  create_new_failure_ = false;
  new_false_lit = true;
#endif
  for (int i = start; i < set_literals_.size(); ++i) {
    clause.push_back(~set_literals_[i]);
    assert(state.assignments_.IsAssignedTrue(set_literals_[i]));

#ifdef USEFAILURE
    if (false_literals_group_.count(~set_literals_[i]) != 0 || false_literals_group_.count(set_literals_[i]) != 0) {
      new_false_lit = true;
    }
#endif
  }


  auto temp = clause[1];
  clause[1] = clause.back();
  clause[clause.size() -1] = temp;


#ifdef USEFAILURE
  total_count++;
  if (new_false_lit) {
    false_literal_ = BooleanLiteral(state.CreateNewVariable(), true);
    false_literals_group_.clear();
    not_false_literals_group_.clear();
    false_literal_weight_ = -1;
  } else {
    reuse_count++;
    std::cout << "Reuse " << reuse_count << " / " << total_count << std::endl;
  }
  for (BooleanLiteral l : set_literals_) {
    not_false_literals_group_.insert(l);
  }
#endif
#ifdef ADDASPERM
  auto res = state.propagator_clausal_.AddPermanentClause(clause, state);
#else
  TwoWatchedClause * learned_clause =  state.AddLearnedClauseToDatabase(clause);
  bool r = state.EnqueuePropagatedLiteral(clause[0], &state.propagator_clausal_, reinterpret_cast<uint64_t>(learned_clause)); //todo can this be done cleaner, without having to refer to how 'code' is interpreted by the propagator?
  assert(r);
  auto res = state.PropagateEnqueued();
#endif
//  state.EnqueuePropagatedLiteral()
  assert(res == NULL);
  assert(state.assignments_.IsAssignedTrue(sumLit));
  auxiliary_literals_.push_back(sumLit);
  auxiliray_boundary_.push_back(set_literals_.size());
}
}
