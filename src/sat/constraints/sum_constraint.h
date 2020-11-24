//
// Created by jens on 20-11-20.
//

#ifndef SIMPLESATSOLVER_SRC_SAT_CONSTRAINTS_SUM_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_SAT_CONSTRAINTS_SUM_CONSTRAINT_H_

#include "../types.h"
#include "i_constraint.h"
#include <vector>
namespace simple_sat_solver::sat {

class SumConstraint : public IConstraint {
public:
  SumConstraint() : input_lits_(), output_lits_(), max_value_(0) {};
  SumConstraint(const std::vector<Lit> &input_lits, const std::vector<Lit> &output_lits)
      : input_lits_(input_lits), output_lits_(output_lits),
        max_value_(output_lits.size()) {};
  SumConstraint(const SumConstraint & constraint) : input_lits_(constraint.input_lits_),
        output_lits_(constraint.output_lits_), max_value_(max_value_) {};
  ~SumConstraint() override = default;
  std::vector<Lit> input_lits_;
  std::vector<Lit> output_lits_;
  int max_value_;

  IConstraint * Clone() override;
};
}
#endif // SIMPLESATSOLVER_SRC_SAT_CONSTRAINTS_SUM_CONSTRAINT_H_
