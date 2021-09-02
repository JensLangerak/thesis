//
// Created by jens on 10-12-20.
//

#include "pseudo_boolean_constraint.h"
#include <cassert>

namespace simple_sat_solver::sat {

PseudoBooleanConstraint::PseudoBooleanConstraint(std::vector<Lit> lits,
                                                 std::vector<int> weights,
                                                 int min, int max) : min_(min), max_(max){
  assert(lits.size() == weights.size());
  for (int i = 0; i < lits.size(); i++) {
    assert(weights[i] >= 0);
    weighted_lits_.push_back(WeightedLit(lits[i], weights[i]));
    for (int j = 0; j < i; ++j) {
      if (lits[i] == lits[j])
        throw "Duplicate lit";
    }
  }

}
PseudoBooleanConstraint::PseudoBooleanConstraint(
    const PseudoBooleanConstraint *constraint) : min_(constraint->min_), max_(constraint->max_), weighted_lits_(constraint->weighted_lits_){
}
IConstraint *PseudoBooleanConstraint::Clone() { return new PseudoBooleanConstraint(*this); }
}