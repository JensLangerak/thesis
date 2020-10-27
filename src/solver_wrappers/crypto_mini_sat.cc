//
// Created by jens on 25-09-20.
//

#include "crypto_mini_sat.h"

#include "../sat/types.h"

namespace simple_sat_solver::solver_wrappers {
using CMSat::SATSolver;
bool CryptoMiniSat::Solve(const sat::SatProblem &p) {

  CMSat::SATSolver solver;
  solver.set_num_threads(4);
  solver.new_vars(p.GetNrVars());
  for (std::vector<sat::Lit> c : p.GetClauses()) {
    std::vector<CMSat::Lit> clause;
    for (sat::Lit l : c)
      clause.emplace_back(l.x, l.complement);

    solver.add_clause(clause);
  }
  auto res = solver.solve();
  if (res == CMSat::l_True) {
    auto res = solver.get_model();
    solution_ = std::vector<bool>();
    for (auto v : res) {
      solution_.push_back(v == CMSat::l_True);
    }
    solved_ = true;
  } else {
    solved_ = false;
  }
  return solved_;
}
std::vector<bool> CryptoMiniSat::GetSolution() const {
  if (!solved_)
    throw "Illegal call to solve";
  return solution_;
}
bool CryptoMiniSat::Optimize(const sat::SatProblem &p) { return false; }
} // namespace simple_sat_solver::solver_wrappers
