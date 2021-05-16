//
// Created by jens on 10-12-20.
//

#include "explanation_pseudo_boolean_constraint.h"
#include "../../../../logger/logger.h"
#include "watched_pseudo_boolean_constraint.h"
#include "../../../Engine/solver_state.h"

namespace Pumpkin {

BooleanLiteral ExplanationPseudoBooleanConstraint2::operator[](int index) const {
  assert(index < lits_.size());
  return lits_[index];
}
BooleanLiteral ExplanationPseudoBooleanConstraint2::operator[](size_t index) const {
  assert(index < lits_.size());
  return lits_[index];
}
ExplanationPseudoBooleanConstraint2::ExplanationPseudoBooleanConstraint2(
    WatchedPseudoBooleanConstraint2 *constraint, SolverState &state) {
  assert(constraint!= nullptr);
  assert(constraint->current_sum_value > constraint->max_);
  constraint->UpdateConflictCount(state);
//  simple_sat_solver::logger::Logger::Log2("Explain Conflict: " + std::to_string(constraint->log_id_));
  std::string cause;
  lits_ = std::vector<BooleanLiteral>();
  // Check if the min or max constraint is violated
  bool select_value = constraint->current_sum_value > constraint->max_;
  int sum = 0;
  for (auto wl : constraint->current_literals_) {
    BooleanLiteral l = wl.literal;
    if (state.assignments_.IsAssigned(l) &&
        state.assignments_.GetAssignment(l) == select_value) {
      lits_.push_back(l);
      sum += wl.weight;
      cause += std::to_string(l.code_) + " ";
    }
  }
//  simple_sat_solver::logger::Logger::Log2("Conflict lits for constraint " + std::to_string(constraint->log_id_)+" : " + cause );

  constraint->UpdateCounts(lits_, state);


  int test = lits_.size();
  // TODO not sure if >= should be possible or that is should be ==
  if (sum < constraint ->max_) {
    assert(sum >= constraint->max_);
  }
//  assert(select_value && lits_.size() >= constraint->current_sum_value);
//      (!select_value) && lits_.size() >= constraint->false_count_);
}
ExplanationPseudoBooleanConstraint2::ExplanationPseudoBooleanConstraint2(
    WatchedPseudoBooleanConstraint2 *constraint, SolverState &state,
    BooleanLiteral propagated_literal) {
  assert(constraint!= nullptr);
  assert(state.assignments_.IsAssignedTrue(propagated_literal));
  constraint->UpdatePropagateCount(state);
//  simple_sat_solver::logger::Logger::Log2("Explain propagation: " + std::to_string(constraint->log_id_));
  // check if true for the minimum value is propagated or false for the upper bound.
  bool propagated_value = true;
  int l_w = 0;
  for (auto wl : constraint->current_literals_) {
    BooleanLiteral l =wl.literal;
    if (l == propagated_literal || ~l == propagated_literal) {
      propagated_value = l == propagated_literal;
      l_w = wl.weight;
      break;
    }
  }

//  assert((!propagated_value) && constraint->current_sum_value >= constraint->max_);
//      propagated_value && constraint->false_count_ >=
//          constraint->literals_.size() - constraint->min_);
  // get the cause for the propagation
  bool cause_value = !propagated_value;
  int propagation_level = state.assignments_.GetTrailPosition(propagated_literal.Variable());
  lits_ = std::vector<BooleanLiteral>();
  int sum = 0;
  std::string cause;
  for (auto wl : constraint->current_literals_) {
    BooleanLiteral l = wl.literal;
    if (state.assignments_.IsAssigned(l) && state.assignments_.GetTrailPosition(l.Variable()) < propagation_level &&
        state.assignments_.GetAssignment(l) == cause_value) {
      lits_.push_back(l);
      sum += wl.weight;
      cause += std::to_string(l.code_) + " ";
    }
  }
//  simple_sat_solver::logger::Logger::Log2("Propagate lit " +std::to_string(propagated_literal.code_)+" for constraint " + std::to_string(constraint->log_id_)+" : " + cause );
  assert(sum + l_w > constraint->max_);
  lits_.push_back(~propagated_literal); //TODO not sure if correct?
  constraint->UpdateCounts(lits_, state);
  // TODO trim lits if to many
}
size_t ExplanationPseudoBooleanConstraint2::Size() const { return lits_.size(); }
} // namespace Pumpkin