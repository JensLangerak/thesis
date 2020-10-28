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
  void TestCardinalityOption(std::string path, int max, solver_wrappers::Pumpkin::CardinalityOption cardinality_option) {
    Parser parser;
    sat::SatProblem problem = parser.Parse(path, max);
    solver_wrappers::Pumpkin solver(cardinality_option);
    std::clock_t start = std::clock();
    bool res = solver.Solve(problem);
    double duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
//    assert(res);
    std::cout << (res ? "T" : "F") << std::endl;
    std::cout<<"Duration: "<< duration <<'\n';
  }
  void TestFile(std::string path, int max) {
    std::cout<<"Encoding" <<std::endl;
    TestCardinalityOption(path, max, solver_wrappers::Pumpkin::CardinalityOption::Encode);
    std::cout<<"Dynamic" <<std::endl;
    TestCardinalityOption(path, max, solver_wrappers::Pumpkin::CardinalityOption::Dynamic);
    std::cout<<"Propagator" <<std::endl;
    TestCardinalityOption(path, max, solver_wrappers::Pumpkin::CardinalityOption::Propagator);

  }
}

int main() {
  simple_sat_solver::cardinality_benchmark::TestFile("/home/jens/Downloads/cc.1/cnf.10.d.1", 9);

}