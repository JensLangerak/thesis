//
// Created by jens on 23-09-20.
//

#include "generator.h"
#include <iostream>

#include "../solver_wrappers/i_solver.h"
#include "../solver_wrappers/simple_solver.h"
#include "../sudoku/encoder.h"
#include "generator_v_2.h"

void TestGenerator();
namespace simple_sat_solver::sudoku_generator {
void PrintSudoku(Sudoku &s) {
  for (int i = 0; i < s.cells.size(); i++) {
    if (s.cells[i] <= 0)
      std::cout << "_ ";
    else
      std::cout << s.cells[i] << " ";
    if ((i + 1) % (s.sub_size * s.sub_size) == 0)
      std::cout << std::endl;
  }
  std::cout << std::endl;
}
void TestGeneratorV2() {

  solver_wrappers::ISolver *solver = new solver_wrappers::SimpleSolver();
  GeneratorV2 g(solver, 3);
  auto res = g.Generate();
  PrintSudoku(res);
}


void TestGenerator() {
    solver_wrappers::ISolver *solver = new solver_wrappers::SimpleSolver();
    Generator g(solver);
    Sudoku s = g.Generate(3, 22);
    PrintSudoku(s);
    sudoku::Encoder encoder(3);
    sat::SatProblem sat = encoder.Encode(s);
    if (solver->Solve(sat)) {
      std::vector<bool> sat_sol = solver->GetSolution();
      Sudoku sol = encoder.Decode(sat_sol);
      PrintSudoku(sol);
    } else {
      throw "Error";
    }

  }

}

int main() {

  simple_sat_solver::sudoku_generator::TestGeneratorV2();
  //simple_sat_solver::sudoku_generator::TestGenerator();
  return 0;
}
