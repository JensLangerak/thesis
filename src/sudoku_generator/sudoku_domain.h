//
// Created by jens on 24-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_SUDOKU_GENERATOR_SUDOKU_DOMAIN_H_
#define SIMPLESATSOLVER_SRC_SUDOKU_GENERATOR_SUDOKU_DOMAIN_H_
#include <vector>
namespace simple_sat_solver::sudoku_generator {
struct SudokuDomain {
  inline explicit SudokuDomain(int sub_size) : sub_size(sub_size) {
    int size = sub_size * sub_size;
    domains = std::vector(size * size, std::vector(size, true));
  }
  int sub_size;
  std::vector<std::vector<bool>> domains;
};
} // namespace simple_sat_solver::sudoku_generator

#endif // SIMPLESATSOLVER_SRC_SUDOKU_GENERATOR_SUDOKU_DOMAIN_H_
