//
// Created by jens on 28-10-20.
//

#include "../pumpkin/Propagators/Cardinality/Encoders/sequential_encoder.h"
#include "../solver_wrappers/pumpkin.h"
#include "../solver_wrappers/simple_solver.h"
#include "parser.h"
#include <cassert>
#include <ctime>
#include <iostream>
namespace simple_sat_solver::cardinality_benchmark {
  void TestCardinalityOption(std::string path, int max, ::Pumpkin::IEncoder::IFactory * encoder_factory) {
    Parser parser;
    sat::SatProblem problem = parser.Parse(path, max);
    solver_wrappers::Pumpkin solver(encoder_factory);
    std::clock_t start = std::clock();
    bool res = solver.Solve(problem);
    double duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
//    assert(res);
    std::cout << (res ? "T" : "F") << std::endl;
    std::cout<<"Duration: "<< duration <<'\n';
  }
  void TestFile(std::string path, int max) {
    Pumpkin::IEncoder::IFactory * encoder_factory;
//    std::cout<<"Encoding - Totalizer" <<std::endl;
//    TestCardinalityOption(path, max, solver_wrappers::Pumpkin::CardinalityOption::Totolizer, true);
    std::cout<<"Encoding - Sequential" <<std::endl;
    encoder_factory = new Pumpkin::SequentialEncoder::Factory();
    encoder_factory->add_dynamic_ = false;
    TestCardinalityOption(path, max, encoder_factory);
//    std::cout<<"Dynamic - Totalizer" <<std::endl;
//    TestCardinalityOption(path, max, solver_wrappers::Pumpkin::CardinalityOption::Totolizer, false);
    std::cout<<"Dynamic - Sequential" <<std::endl;
    encoder_factory = new Pumpkin::SequentialEncoder::Factory();
    encoder_factory->add_dynamic_ = true;
    TestCardinalityOption(path, max, encoder_factory);
//    std::cout<<"Propagator" <<std::endl;
//    TestCardinalityOption(path, max, solver_wrappers::Pumpkin::CardinalityOption::Propagator, false);

  }
}

int main() {
  simple_sat_solver::cardinality_benchmark::TestFile("/home/jens/Downloads/cc.1/cnf.12.t.1", 11);


}