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
  static Sudoku Parse(std::string file);
};
}

#endif // SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_BENCHMARK_PARSER_H_
