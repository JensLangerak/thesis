//
// Created by jens on 16-03-21.
//

#include "explanation_pb_sum_constraint.h"

namespace Pumpkin {

BooleanLiteral ExplanationPbSumConstraint::operator[](int index) const {
  assert(index < lits_.size());
  return lits_[index];
}
BooleanLiteral ExplanationPbSumConstraint::operator[](size_t index) const {
  assert(index < lits_.size());
  return lits_[index];
}
ExplanationPbSumConstraint::ExplanationPbSumConstraint(
    WatchedPbSumConstraint *constraint, SolverState &state) {
  assert(constraint!= nullptr);
  WeightedLiteral wl;
  bool found = false;
  lits_ = std::vector<BooleanLiteral>();



//  int sum = 0;
//  for (WeightedLiteral l : constraint->inputs_) {
//    if (state.assignments_.IsAssignedTrue(l.literal)) {
//      sum += l.weight;
//      lits_.push_back(l.literal);
//    }
//  }
//  BooleanLiteral out;
//  for (WeightedLiteral l : constraint->outputs_) {
//    if (l.weight > sum)
//      continue;
//    if (state.assignments_.IsAssignedFalse(l.literal)) {
////      out = ~l.literal;
//      lits_.push_back(~l.literal);
//      return;
//    }
//  }
//  lits_.push_back(out);
//
//  return;



//  for (int i = constraint->weight_trail_delimitors_.back(); i < constraint->weight_trail_.size(); ++i) {
    for (int i = constraint->weight_trail_.size() - 1; i >= 0; --i) {
    int w = constraint->weight_trail_[i];
    if (constraint->weight_output_index_map_.count(w) > 0) {
      int index = constraint->weight_output_index_map_.at(w);
      wl = constraint->outputs_[index];
      if (state.assignments_.IsAssignedFalse(wl.literal)) {
        found = true;
        lits_.push_back(~wl.literal);
        break;
      }
    }
  }
  assert(found);
//
//  return;

  int w = wl.weight;
  bool use_max = false;
  if (w >= constraint->max_) {
    w = constraint->max_cause_weight;
    use_max =true;
  }
  while (w > 0) {
    int ii = 0;
    if (use_max) {
      ii = constraint->weight_cause_index_.at(constraint->max_);
      use_max = false;
    } else {
      ii = constraint->weight_cause_index_.at(w);
    }

    WeightedLiteral input = constraint->inputs_[ii];
    w -= input.weight;
    assert(state.assignments_.IsAssignedTrue(input.literal));
    lits_.push_back(input.literal);
  }
  assert(w == 0);
  for (BooleanLiteral l : lits_) {
    assert(state.assignments_.IsAssignedTrue(l));
  }
  constraint->UpdateCounts(lits_, state);
}
ExplanationPbSumConstraint::ExplanationPbSumConstraint(
     WatchedPbSumConstraint *constraint, SolverState &state,
    BooleanLiteral propagated_literal) {
  if (constraint->lit_to_output_index_.count(propagated_literal.ToPositiveInteger()) == 0) {
    // INPUT
    int sum = 0;
    for (auto l : constraint->inputs_) {
      if (l.literal == ~propagated_literal) {
        int segs= 2;
      }
      if (state.assignments_.IsAssignedTrue(l.literal) && state.assignments_.GetTrailPosition(propagated_literal.Variable()) > state.assignments_.GetTrailPosition(l.literal.Variable())) {
        lits_.push_back(l.literal);
        sum += l.weight;
      } else if (l.literal == ~propagated_literal) {
        sum += l.weight;
      }
    }
    for (auto l : constraint->outputs_) {
      if (state.assignments_.IsAssignedFalse(l.literal)) {
        if (l.weight <= sum) {
          lits_.push_back(~l.literal);
          break;
        }
      }
    }
//    assert(false);

  } else {
    // OUTPUT
    int wi = constraint->lit_to_output_index_.at(
        propagated_literal.ToPositiveInteger());
    WeightedLiteral wl = constraint->outputs_[wi];
    int w = wl.weight;
    while (w > 0) {
      int ii = constraint->weight_cause_index_.at(w);
      WeightedLiteral input = constraint->inputs_[ii];
      w -= input.weight;
      lits_.push_back(input.literal);
    }
  }
  constraint->UpdateCounts(lits_, state);

}
size_t ExplanationPbSumConstraint::Size() const { return lits_.size(); }

}