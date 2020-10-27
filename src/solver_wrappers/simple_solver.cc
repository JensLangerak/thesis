//
// Created by jens on 18-09-20.
//

#include "simple_solver.h"
#include "../sat/encoders/totaliser_encoder.h"
#include "../solver/solver.h"

namespace simple_sat_solver::solver_wrappers {

bool SimpleSolver::Solve(const sat::SatProblem &p2) {
  solved_ = false;
  sat::SatProblem p = p2;
  for (sat::CardinalityConstraint c : p.GetConstraints()) {
    sat::TotaliserEncoder::Encode(p, c.lits, c.min, c.max);
  }
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
bool SimpleSolver::Optimize(const sat::SatProblem &p) { return false; }
} // namespace simple_sat_solver::solver_wrappers
