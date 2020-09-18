//
// Created by jens on 18-09-20.
//

#include "simple_solver.h"
#include "../solver/solver.h"

namespace simple_sat_solver::solver_wrappers {

bool SimpleSolver::Solve(const sat::SatProblem &p) {
  solved_ = false;
  solver::Solver s;
  for (int i = 0; i < p.GetNrVars(); i++)
    s.NewVar();
  for (const auto &c : p.GetClauses())
    s.AddClause(c);

  solved_ = s.Solve();
  if (solved_)
    solution_ = s.GetModel();

  return solved_;
}
std::vector<bool> SimpleSolver::GetSolution() const { return solution_; }
} // namespace simple_sat_solver::solver_wrappers
