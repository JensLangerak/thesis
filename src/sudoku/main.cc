//
// Created by jens on 17-09-20.
//

#include <filesystem>
#include <iostream>

#include "../sat/sat_problem.h"
#include "../solver_wrappers/i_solver.h"
#include "../solver_wrappers/simple_solver.h"
#include "benchmark_parser.h"
#include "encoder.h"

namespace simple_sat_solver::sudoku {
void PrintSudoku(Sudoku &s) {
  for (int i = 0; i < s.cells.size(); i++) {
    std::cout << s.cells[i] << " ";
    if ((i + 1) % (s.sub_size * s.sub_size) == 0)
      std::cout << std::endl;
  }
  std::cout << std::endl;
}

bool SolutionValidForInput(const Sudoku &solution, const Sudoku &input) {
  if (solution.sub_size != input.sub_size)
    return false;
  for (int i = 0; i < solution.cells.size(); i++) {
    if (input.cells[i] < 1) // filled in cell
      continue;
    if (input.cells[i] != solution.cells[i])
      return false;
  }
  return true;
}

bool TestSudoku(std::string path) {
  Sudoku sudoku = BenchmarkParser::Parse(path);
  sat::SatProblem p = Encoder(sudoku.sub_size).Encode(sudoku);
  solver_wrappers::ISolver *s = new solver_wrappers::SimpleSolver();
  bool res = s->Solve(p);
  std::vector<bool> sat_solution = s->GetSolution();
  delete s;

  Sudoku solution = Encoder(sudoku.sub_size).Decode(sat_solution);

  if (!SolutionValidForInput(solution, sudoku))
    throw "Wrong result";
  return res;
}
void SingleFile(const std::string &path) {
  Sudoku sudoku = BenchmarkParser::Parse(path);
  sat::SatProblem p = Encoder(sudoku.sub_size).Encode(sudoku);
  solver_wrappers::ISolver *s = new solver_wrappers::SimpleSolver();
  if (s->Solve(p)) {
    std::cout << "Solved!" << std::endl;
    std::vector<bool> sat_solution = s->GetSolution();
    Sudoku solution = Encoder(sudoku.sub_size).Decode(sat_solution);
    if (!SolutionValidForInput(solution, sudoku)) {
      std::cout << "Error" << std::endl;
    }
    PrintSudoku(solution);

  } else {
    std::cout << "Not solvable" << std::endl;
  }
  delete s;
}

void AllBenchmarksInFolder(const std::string &path) {
  for (const auto &sudoku_entry : std::filesystem::directory_iterator(path)) {
    std::cout << sudoku_entry.path() << std::endl;
    if (TestSudoku(sudoku_entry.path())) {
      std::cout << sudoku_entry.path() << "   SOLVED " << std::endl;
    } else {
      std::cout << sudoku_entry.path() << "   NOT SOLVED " << std::endl;
    }
  }
}
void AllBenchmarks() {
  std::cout << "Start benchmark" << std::endl;
  std::string base_dir = "../../../data/sudoku/"; // TODO use base dir
  int no_solution = 0;
  int solved = 0;
  for (const auto &set_entry : std::filesystem::directory_iterator(base_dir)) {
    std::cout << set_entry.path() << std::endl;
    for (const auto &subset_entry :
         std::filesystem::directory_iterator(set_entry)) {
      std::cout << "      " << subset_entry.path() << std::endl;

      for (const auto &sudoku_entry :
           std::filesystem::directory_iterator(subset_entry.path())) {
        if (TestSudoku(sudoku_entry.path())) {
          // std::cout << "             " << sudoku_entry.path() << "   SOLVED "
          //           << std::endl;
          ++solved;
        } else {
          ++no_solution;
        }
      }
    }
  }
  int total = no_solution + solved;
  std::cout << std::endl;
  std::cout << "Solved: " << solved << " / " << total << std::endl;
}
}

int main() {
  // SingleFile("../../../data/sudoku/benchmarks3x3/10/puzzle5.txt");
  // SingleFile("../../../data/sudoku/benchmarks3x3/20/puzzle5.txt");

  // AllBenchmarksInFolder("../../../data/sudoku/benchmarks5x5/55/");
  simple_sat_solver::sudoku::AllBenchmarks();
  return 0;
}
