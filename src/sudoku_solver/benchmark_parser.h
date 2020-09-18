//
// Created by jens on 16-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_BENCHMARK_PARSER_H_
#define SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_BENCHMARK_PARSER_H_

#include "types.h"
#include <string>
namespace simple_sat_solver::sudoku {
class BenchmarkParser {
public:
  /// Read a sudoku file and construct a sudoku.
  /// \param file the file that contains the sudoku.
  /// \return a sudoku.
  static Sudoku Parse(const std::string &file);
};
} // namespace simple_sat_solver::sudoku

#endif // SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_BENCHMARK_PARSER_H_
