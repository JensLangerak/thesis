//
// Created by jens on 19-11-20.
//

#ifndef SIMPLESATSOLVER_SRC_SAT_CONSTRAINTS_CARDINALITY_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_SAT_CONSTRAINTS_CARDINALITY_CONSTRAINT_H_

#include "../types.h"
#include "i_constraint.h"
#include <vector>
namespace simple_sat_solver::sat {
class CardinalityConstraint : public IConstraint {
public:
  CardinalityConstraint() : lits(), min(0), max(0){};
  CardinalityConstraint(const std::vector<Lit> &lits, int min, int max)
      : lits(lits), min(min), max(max){};
  CardinalityConstraint(const CardinalityConstraint & constraint) : lits(constraint.lits), max(constraint.max), min(constraint.min){};
  ~CardinalityConstraint() override;
  std::vector<Lit> lits;
  int min;
  int max;

  IConstraint * Clone() override;
};
} // namespace simple_sat_solver::sat

#endif // SIMPLESATSOLVER_SRC_SAT_CONSTRAINTS_CARDINALITY_CONSTRAINT_H_
