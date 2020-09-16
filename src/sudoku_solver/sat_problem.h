//
// Created by jens on 16-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_SAT_PROBLEM_H_
#define SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_SAT_PROBLEM_H_

#include <vector>

#include "../solver/types.h"

namespace simple_sat_solver::sudoku {
struct SatProblem {
  int nr_vars;
  std::vector<std::vector<solver::Lit>> clauses;

  SatProblem() : nr_vars(0) {};
  SatProblem(int nr_vars) : nr_vars(nr_vars) {};
};
}
#endif // SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_SAT_PROBLEM_H_
