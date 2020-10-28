//
// Created by jens on 28-10-20.
//

#include "../solver_wrappers/pumpkin.h"
#include "../solver_wrappers/simple_solver.h"
#include "parser.h"
#include <cassert>
#include <ctime>
#include <iostream>
namespace simple_sat_solver::cardinality_benchmark {
  void TestCardinalityOption(std::string path, int max, solver_wrappers::Pumpkin::CardinalityOption cardinality_option, bool add_encoding) {
    Parser parser;
    sat::SatProblem problem = parser.Parse(path, max);
    solver_wrappers::Pumpkin solver(cardinality_option, add_encoding);
    std::clock_t start = std::clock();
    bool res = solver.Solve(problem);
    double duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
//    assert(res);
    std::cout << (res ? "T" : "F") << std::endl;
    std::cout<<"Duration: "<< duration <<'\n';
  }
  void TestFile(std::string path, int max) {
    std::cout<<"Encoding - Totalizer" <<std::endl;
    TestCardinalityOption(path, max, solver_wrappers::Pumpkin::CardinalityOption::Totolizer, true);
    std::cout<<"Encoding - Sequential" <<std::endl;
    TestCardinalityOption(path, max, solver_wrappers::Pumpkin::CardinalityOption::Sequential, true);
    std::cout<<"Dynamic - Totalizer" <<std::endl;
    TestCardinalityOption(path, max, solver_wrappers::Pumpkin::CardinalityOption::Totolizer, false);
    std::cout<<"Dynamic - Sequential" <<std::endl;
    TestCardinalityOption(path, max, solver_wrappers::Pumpkin::CardinalityOption::Sequential, false);
    std::cout<<"Propagator" <<std::endl;
    TestCardinalityOption(path, max, solver_wrappers::Pumpkin::CardinalityOption::Propagator, false);

  }
}

int main() {
  simple_sat_solver::cardinality_benchmark::TestFile("/home/jens/Downloads/cc.5/cnf.20.t.5", 20);

}