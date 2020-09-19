//
// Created by jens on 18-09-20.
//
#include <iostream>
#include <string>

#include "b2b_converter.h"
#include "b2b_parser.h"
#include "../solver_wrappers/simple_solver.h"
namespace simple_sat_solver::b2b {
  void TestFile(std::string path) {
    std::cout << "Test: " << path << std::endl;
    B2B problem = B2bParser::Parse(path);
    B2bConverter converter(problem);
    sat::SatProblem sat = converter.ToSat();
    solver_wrappers::ISolver * solver = new solver_wrappers::SimpleSolver();
    bool res = solver->Solve(sat);
    if (res) {
      std::vector<int> schedule = converter.DecodeSolution(solver->GetSolution());
      std::cout << "Solved" << std::endl;
    } else {
      std::cout << "Unsolvable" << std::endl;
    }
  }
}

int main() {
  simple_sat_solver::b2b::TestFile("../../../data/b2b/b2bsat/dzn/forum-14.dzn");
}