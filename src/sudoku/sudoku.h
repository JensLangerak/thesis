//
// Created by jens on 16-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_TYPES_H_
#define SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_TYPES_H_

#include <utility>
#include <vector>

#include "../solver/types.h"

namespace simple_sat_solver::sudoku {
struct Sudoku {
  int sub_size;
  std::vector<int> cells;

  inline Sudoku() : sub_size(0), cells(){};
  inline Sudoku(int sub_size, std::vector<int> cells)
      : sub_size(sub_size), cells(std::move(cells)){};
};

inline bool operator==(const Sudoku &lhs, const Sudoku &rhs) {
  return lhs.sub_size == rhs.sub_size && lhs.cells == rhs.cells;
}
} // namespace simple_sat_solver::sudoku
#endif // SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_TYPES_H_
