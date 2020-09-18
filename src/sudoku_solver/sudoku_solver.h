//
// Created by jens on 16-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_SUDOKU_SOLVER_H_
#define SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_SUDOKU_SOLVER_H_

#include <vector>

#include "types.h"

namespace simple_sat_solver::sudoku {
class SudokuSolver {
  /// Class that uses the simple_sat_solver::solver to solver a a sat problem
public:
  inline SudokuSolver() : solved_(false){};
  /// Return true when p is satisfiable, if unsatisfiable it returns false.
  /// After returning true, the solution can be obtained with GetSolution.
  /// \param p
  /// \return
  bool Solve(const SatProblem &p);

  /// Return the solution found be solve. Should only be used after Solver
  /// returns true.
  /// \return the solution found by solve.
  std::vector<bool> GetSolution();

private:
  std::vector<bool> solution_;
  bool solved_;
};
} // namespace simple_sat_solver::sudoku

#endif // SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_SUDOKU_SOLVER_H_
