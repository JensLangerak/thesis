//
// Created by jens on 18-09-20.
//

#include "sat_problem.h"
#include <algorithm>
namespace simple_sat_solver::sat {
void SatProblem::AddClause(const std::vector<Lit> &lits) {
  clauses_.push_back(lits);
}
void SatProblem::AtMostOne(const std::vector<Lit> &lits) {
  for (int i = 0; i < lits.size() - 1; i++) {
    for (int j = i + 1; j < lits.size(); j++)
      clauses_.push_back({~lits[i], ~lits[j]});
  }
}
void SatProblem::ExactlyOne(const std::vector<Lit> &lits) {
  AtLeastOne(lits);
  AtMostOne(lits);
}
bool SatProblem::TestLit(const Lit &l, const std::vector<bool> &vars) {
  return (!l.complement) == vars[l.x];
}
bool SatProblem::TestClause(const std::vector<Lit> &c,
                            const std::vector<bool> &vars) {
  return std::any_of(c.begin(), c.end(),
                     [vars](const Lit &l) { return TestLit(l, vars); });
}
bool SatProblem::TestAssignment(const std::vector<bool> &vars) const {
  if (vars.size() != nr_vars_)
    throw "Number of vars differ";

  return std::all_of(
      clauses_.begin(), clauses_.end(),
      [vars](const std::vector<Lit> &c) { return TestClause(c, vars); });
};
} // namespace simple_sat_solver::sat
