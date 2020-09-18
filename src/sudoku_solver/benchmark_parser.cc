//
// Created by jens on 16-09-20.
//

#include "benchmark_parser.h"

#include <fstream>
#include <iostream>

#include "encoder.h"
namespace simple_sat_solver::sudoku {
Sudoku BenchmarkParser::Parse(const std::string &path) {
  std::ifstream file_stream(path);
  if (!file_stream.is_open()) {
    std::cout << "Cannot open file: " << path << std::endl;
    file_stream.close();
    throw "Cannot open file";
  }
  int sub_size;
  file_stream >> sub_size;
  int value;
  file_stream >> value;

  std::vector<int> numbers = std::vector<int>();
  while (file_stream >> value) {
    numbers.push_back(value);
  }

  file_stream.close();
  return Sudoku(sub_size, numbers);
}
} // namespace simple_sat_solver::sudoku
