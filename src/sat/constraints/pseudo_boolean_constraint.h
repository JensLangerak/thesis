//
// Created by jens on 10-12-20.
//

#ifndef SIMPLESATSOLVER_SRC_SAT_CONSTRAINTS_PSEUDO_BOOLEAN_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_SAT_CONSTRAINTS_PSEUDO_BOOLEAN_CONSTRAINT_H_

#include "../types.h"
#include "i_constraint.h"
#include <vector>

namespace simple_sat_solver::sat {
class PseudoBooleanConstraint : public IConstraint {
public:
  PseudoBooleanConstraint(std::vector<Lit> lits, std::vector<int> weights, int min, int max);
  explicit PseudoBooleanConstraint(const PseudoBooleanConstraint * constraint);
  ~PseudoBooleanConstraint() override {};
  IConstraint * Clone() override;

  int min_;
  int max_;

  std::vector<WeightedLit> weighted_lits_;



};
}
#endif // SIMPLESATSOLVER_SRC_SAT_CONSTRAINTS_PSEUDO_BOOLEAN_CONSTRAINT_H_
