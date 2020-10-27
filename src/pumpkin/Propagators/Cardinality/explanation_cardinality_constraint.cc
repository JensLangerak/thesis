//
// Created by jens on 17-10-20.
//

#include "explanation_cardinality_constraint.h"
namespace Pumpkin {

BooleanLiteral ExplanationCardinalityConstraint::operator[](int index) const {
  assert(index < lits_.size());
  return lits_[index];
}
BooleanLiteral ExplanationCardinalityConstraint::operator[](size_t index) const {
  assert(index < lits_.size());
  return lits_[index];
}
ExplanationCardinalityConstraint::ExplanationCardinalityConstraint(
    const WatchedCardinalityConstraint *constraint, SolverState &state) {
  assert(constraint!= nullptr);
  assert(constraint->true_count_ > constraint->max_ ||
         constraint->false_count_ >
             constraint->literals_.size() - constraint->min_);
  lits_ = std::vector<BooleanLiteral>();
  bool select_value = constraint->true_count_ > constraint->max_;
  for (BooleanLiteral l : constraint->literals_) {
    if (state.assignments_.IsAssigned(l) &&
        state.assignments_.GetAssignment(l) == select_value)
      lits_.push_back(l);
  }

//  if (select_value && lits_.size() > constraint->true_count_) {
//    std::vector<BooleanLiteral> lits2;
//    int min_trail_level = -1;
//    int min_trail_level_current = -1;
//    int min_index = -1;
//    while(lits2.size() <= constraint->true_count_) {
//      for (int i = 0; i < lits_.size(); ++i) {
//        BooleanLiteral l = lits_[i];
//        int trail_pos = state.assignments_.GetTrailPosition(lits_[0].Variable());
//        if (trail_pos > min_trail_level) {
//          if (min_index == -1) {
//            min_index = i;
//            min_trail_level_current = trail_pos;
//          } else if (min_trail_level_current > trail_pos) {
//            min_index = i;
//            min_trail_level_current = trail_pos;
//          }
//        }
//      }
//      lits2.push_back(lits_[min_index]);
//    }
//
//    lits_ = lits2;
//  }

  int test = lits_.size();
  // TODO not sure if >= should be possible or that is should be ==
  assert(select_value && lits_.size() >= constraint->true_count_ ||
         (!select_value) && lits_.size() >= constraint->false_count_);
}
ExplanationCardinalityConstraint::ExplanationCardinalityConstraint(
    const WatchedCardinalityConstraint *constraint, SolverState &state,
    BooleanLiteral propagated_literal) {
  assert(constraint!= nullptr);
  assert(state.assignments_.IsAssigned(propagated_literal));
  bool propagated_value = true;
  assert(state.assignments_.IsAssignedTrue(propagated_literal));
  for (BooleanLiteral l : constraint->literals_) {
    if (l == propagated_literal || ~l == propagated_literal) {
      propagated_value = l == propagated_literal;
      break;
    }
  }

  assert((!propagated_value) && constraint->true_count_ >= constraint->max_ ||
         propagated_value && constraint->false_count_ >=
                                 constraint->literals_.size() - constraint->min_);
  bool cause_value = !propagated_value;

  int propagation_level = state.assignments_.GetTrailPosition(propagated_literal.Variable());
  lits_ = std::vector<BooleanLiteral>();
  for (BooleanLiteral l : constraint->literals_) {
    if (state.assignments_.IsAssigned(l) && state.assignments_.GetTrailPosition(l.Variable()) < propagation_level &&
        state.assignments_.GetAssignment(l) == cause_value)
      lits_.push_back(l);
  }
}
size_t ExplanationCardinalityConstraint::Size() const { return lits_.size(); }
} // namespace Pumpkin