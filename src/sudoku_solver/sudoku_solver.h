//
// Created by jens on 16-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_SUDOKU_SOLVER_H_
#define SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_SUDOKU_SOLVER_H_

#include <vector>

#include "types.h"

namespace simple_sat_solver::sudoku {
class SudokuSolver {
public:
  inline SudokuSolver() : solved_(false) {};
  bool Solve(const SatProblem &p);
  std::vector<bool> GetSolution();

private:
  std::vector<bool> solution_;
  bool solved_;
};
}

#endif // SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_SUDOKU_SOLVER_H_
