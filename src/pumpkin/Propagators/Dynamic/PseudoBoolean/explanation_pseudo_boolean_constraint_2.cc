//
// Created by jens on 24-05-21.
//

#include "explanation_pseudo_boolean_constraint_2.h"
#include "../../../Engine/solver_state.h"
#include "../../../Utilities/boolean_variable.h"
#include "../../../Utilities/problem_specification.h"
#include "../Encoders/generalized_totaliser.h"
#define IMPROVED_EXPLANATION
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
#ifdef IMPROVED_EXPLANATION
  bool found_current_decision = false;
  for (auto l : constraint->unencoded_constraint_literals_) {
      if (state.assignments_.IsAssigned(l) &&
          state.assignments_.GetAssignment(l) == select_value) {
        lits_.push_back(l);
        int w = constraint->lit_weights_[l];
        sum +=w;
        if (state.assignments_.GetAssignmentLevel(l) == state.GetCurrentDecisionLevel())
          found_current_decision = true;
      }
  }
  if (!found_current_decision) {
    lits_.clear();
    for (auto wl : constraint->current_literals_) {
      BooleanLiteral l = wl.literal;
      if (state.assignments_.IsAssigned(l) &&
          state.assignments_.GetAssignment(l) == select_value) {
        lits_.push_back(l);
        sum += wl.weight;
      }
    }
  } else {

    GeneralizedTotaliser *encoder =
        dynamic_cast<GeneralizedTotaliser *>(constraint->encoder_);
    if (encoder == nullptr)
      throw "ERROR cast";
    if (encoder->root_ != nullptr) {
      assert(constraint->unencoded_constraint_literals_.size() !=
             constraint->current_literals_.size());
      int max_weight = 0;
      int max_index = -1;

      for (int i = 0; i < encoder->root_->counting_variables.size(); ++i) {
        if (state.assignments_.IsAssignedTrue(
                encoder->root_->counting_variables[i])) {
          if (encoder->root_->counting_variables_weights[i] > max_weight ||
              max_index < 0) {
            max_index = i;
            max_weight = encoder->root_->counting_variables_weights[i];
          }
        }
      }
      if (max_index >= 0) {
        sum += max_weight;
        lits_.push_back(encoder->root_->counting_variables[max_index]);
      }
    }
  }
#else
  for (auto wl : constraint->current_literals_) {
    BooleanLiteral l = wl.literal;
    if (state.assignments_.IsAssigned(l) &&
        state.assignments_.GetAssignment(l) == select_value) {
      lits_.push_back(l);
      sum += wl.weight;
    }
  }
#endif
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
  int l_w = constraint->lit_weights_[(~propagated_literal)];

  int propagation_level =
      state.assignments_.GetTrailPosition(propagated_literal.Variable());
  auto lits_ = std::vector<BooleanLiteral>();
  int sum = 0;
#ifdef IMPROVED_EXPLANATION

  bool found_current_decision = false;
  for (auto l : constraint->unencoded_constraint_literals_) {
    if (state.assignments_.IsAssignedTrue(l) &&
      state.assignments_.GetTrailPosition(l.Variable()) < propagation_level
        ){
      lits_.push_back(l);
      int w = constraint->lit_weights_[l];
      sum +=w;
      if (state.assignments_.GetAssignmentLevel(l) == state.GetCurrentDecisionLevel())
        found_current_decision = true;
    }
  }
  if (!found_current_decision) {
    lits_.clear();
    for (auto wl : constraint->current_literals_) {
      BooleanLiteral l = wl.literal;
      if (state.assignments_.IsAssigned(l) &&
          state.assignments_.GetTrailPosition(l.Variable()) < propagation_level &&
          state.assignments_.IsAssignedTrue(l)) {
        lits_.push_back(l);
        sum += wl.weight;
      }
    }
  }else {
  GeneralizedTotaliser * encoder =
      dynamic_cast<GeneralizedTotaliser *>(constraint->encoder_);
  if (encoder == nullptr)
    throw  "ERROR cast";
  if (encoder->root_ != nullptr) {

    int max_weight = 0;
    int max_index = -1;

    for (int i = 0; i < encoder->root_->counting_variables.size(); ++i) {
      if (state.assignments_.IsAssignedTrue(
              encoder->root_->counting_variables[i]) &&
          state.assignments_.GetTrailPosition(
              encoder->root_->counting_variables[i].Variable()) <
              propagation_level) {
        if (encoder->root_->counting_variables_weights[i] > max_weight ||
            max_index < 0) {
          max_index = i;
          max_weight = encoder->root_->counting_variables_weights[i];
        }
      }
    }
    if (max_index >= 0) {
      sum += max_weight;
      lits_.push_back(encoder->root_->counting_variables[max_index]);
    }
  }

  }
#else
  for (auto wl : constraint->current_literals_) {
    BooleanLiteral l = wl.literal;
    if (state.assignments_.IsAssigned(l) &&
        state.assignments_.GetTrailPosition(l.Variable()) < propagation_level &&
        state.assignments_.IsAssignedTrue(l)) {
      lits_.push_back(l);
      sum += wl.weight;
    }
  }
#endif
  assert(sum + l_w > constraint->max_);
//  lits_.push_back(~propagated_literal); // TODO not sure if correct?
  explanation->Init(lits_);
  constraint->UpdateCounts(lits_, state);
}

} // namespace Pumpkin
