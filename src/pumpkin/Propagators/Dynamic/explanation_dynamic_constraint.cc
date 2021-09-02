//
// Created by jens on 20-05-21.
//

#include "explanation_dynamic_constraint.h"
#include <cassert>
namespace Pumpkin {
BooleanLiteral ExplanationDynamicConstraint::operator[](int index) const {
  assert(index < lits_.size());
  return lits_[index];
}
BooleanLiteral ExplanationDynamicConstraint::operator[](size_t index) const {
  assert(index < lits_.size());
  return lits_[index];
}

size_t ExplanationDynamicConstraint::Size() const { return lits_.size(); }
ExplanationDynamicConstraint::ExplanationDynamicConstraint() {
  lits_ = std::vector<BooleanLiteral>();
}


}
