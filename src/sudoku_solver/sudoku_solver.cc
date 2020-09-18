//
// Created by jens on 16-09-20.
//

#include "sudoku_solver.h"
#include "../solver/solver.h"

namespace simple_sat_solver::sudoku {

bool SudokuSolver::Solve(const SatProblem &p) {
  solved_ = false;
  solver::Solver s;
  for (int i = 0; i < p.nr_vars; i++)
    s.NewVar();
  for (const auto& c : p.clauses)
    s.AddClause(c);

  solved_ = s.Solve();
  if (solved_)
    solution_ = s.GetModel();

  return solved_;
}
std::vector<bool> SudokuSolver::GetSolution() { return solution_; }
} // namespace simple_sat_solver::sudoku
