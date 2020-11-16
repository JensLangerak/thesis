//
// Created by jens on 28-10-20.
//

#include "../pumpkin/Propagators/Cardinality/Encoders/incremental_sequential_encoder.h"
#include "../pumpkin/Propagators/Cardinality/Encoders/propagator_encoder.h"
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
    std::cout <<"-----TEST: " << path << "----------" << std::endl;
    Pumpkin::IEncoder::IFactory * encoder_factory;
//    std::cout<<"Encoding - Totalizer" <<std::endl;
//    TestCardinalityOption(path, max, solver_wrappers::Pumpkin::CardinalityOption::Totolizer, true);
    std::cout<<"Encoding - Sequential" <<std::endl;
    encoder_factory = new Pumpkin::SequentialEncoder::Factory();
    encoder_factory->add_dynamic_ = false;
//    TestCardinalityOption(path, max, encoder_factory);
//    std::cout<<"Dynamic - Totalizer" <<std::endl;
//    TestCardinalityOption(path, max, solver_wrappers::Pumpkin::CardinalityOption::Totolizer, false);
    std::cout<<"Dynamic - Sequential" <<std::endl;
    encoder_factory = new Pumpkin::SequentialEncoder::Factory();
    encoder_factory->add_dynamic_ = true;
//    TestCardinalityOption(path, max, encoder_factory);
    std::cout<<"Incremental - Sequential" <<std::endl;
    encoder_factory = new Pumpkin::IncrementalSequentialEncoder::Factory();
    encoder_factory->add_dynamic_ = true;
    TestCardinalityOption(path, max, encoder_factory);
    std::cout<<"Propagator" <<std::endl;
    encoder_factory = new Pumpkin::PropagatorEncoder::Factory();
    encoder_factory->add_dynamic_ = false;
//    TestCardinalityOption(path, max, encoder_factory);

  }
}

int main() {
  std::string x = std::to_string(1);
  std::string y = "t";
  for (int i = 2; i <= 20; i++) {
    simple_sat_solver::cardinality_benchmark::TestFile(
        "/home/jens/Downloads/cc." + x +"/cnf." + std::to_string(i) + "." +y +"." + x, i);
  }
//  for (int i = 3; i <= 20; i++) {
//    simple_sat_solver::cardinality_benchmark::TestFile(
//        "/home/jens/Downloads/cc." + x +"/cnf." + std::to_string(i) + "." +y +"." + x, i-1);
//  }

}