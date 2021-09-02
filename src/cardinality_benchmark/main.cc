//
// Created by jens on 28-10-20.
//

#include "../logger/logger.h"
#include "../pumpkin/Propagators/Dynamic/Encoders/generalized_totaliser.h"
#include "../pumpkin/Propagators/Dynamic/Encoders/incremental_sequential_encoder.h"
#include "../pumpkin/Propagators/Dynamic/Encoders/propagator_encoder.h"
#include "../pumpkin/Propagators/Dynamic/Encoders/sequential_encoder.h"
#include "../solver_wrappers/pumpkin.h"
#include "../solver_wrappers/simple_solver.h"
#include "parser.h"
#include <cassert>
#include <chrono>
#include <ctime>
#include <iostream>
namespace simple_sat_solver::cardinality_benchmark {
  void TestCardinalityOption(std::string path, int max, ::Pumpkin::IEncoder<::Pumpkin::CardinalityConstraint>::IFactory * encoder_factory) {
    Parser parser;
    assert(encoder_factory != nullptr);
    sat::SatProblem* problem = parser.Parse(path, max);
    solver_wrappers::Pumpkin solver(encoder_factory, 1000);
    std::clock_t start = std::clock();
    bool res = solver.Solve(*problem);
    double duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
//    assert(res);
    std::cout << (res ? "T" : "F") << std::endl;
    std::cout<<"Duration: "<< duration <<'\n';
//    delete encoder_factory;
    delete problem;
  }
  void TestFile(std::string path, int max) {
    std::cout <<"-----TEST: " << path << "----------" << std::endl;
    Pumpkin::IEncoder<::Pumpkin::CardinalityConstraint>::IFactory * encoder_factory;
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
    std::cout<<"Incremental - Sequential" <<std::endl;
    encoder_factory = new Pumpkin::IncrementalSequentialEncoder::Factory();
    encoder_factory->add_dynamic_ = true;
    TestCardinalityOption(path, max, encoder_factory);
    std::cout<<"Propagator" <<std::endl;
    encoder_factory = new Pumpkin::PropagatorEncoder<::Pumpkin::CardinalityConstraint>::Factory();
    encoder_factory->add_dynamic_ = false;
    TestCardinalityOption(path, max, encoder_factory);

  }

void Test(std::string test_file_path, std::string test_file, std::string log_dir, ::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory * encoder, std::string encoder_message, int start_penalty) {
  simple_sat_solver::logger::Logger::StartNewLog(log_dir,test_file);
  simple_sat_solver::logger::Logger::Log2("File: " + test_file_path);
  simple_sat_solver::logger::Logger::Log2("Encoder: " + encoder_message);
  std::string t = "T";
  std::string f = "F";
  simple_sat_solver::logger::Logger::Log2("Dynamic: " + (encoder->add_dynamic_ ? t : f));
  simple_sat_solver::logger::Logger::Log2("Incremental: " + (encoder->add_incremetal_ ? t : f));

  solver_wrappers::ISolver * solver = new solver_wrappers::Pumpkin(encoder, start_penalty);

  Parser parser;
  sat::SatProblem* problem = parser.Parse(test_file_path, -1);
  std::clock_t start = std::clock();
  bool res = solver->Optimize(*problem);
  double duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  std::cout << "time: " << duration <<std::endl;
  simple_sat_solver::logger::Logger::Log2("Time: " + std::to_string(duration));
//    assert(res);

  delete problem;
  delete solver;
  simple_sat_solver::logger::Logger::End();
}
}

void test_setting(std::string test_file_path, std::string test_file, std::string log_dir,
                  Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory *encoder_factory,
                  std::string encoder_string, bool add_dynamic,
                  bool add_incremental, int start_penalty) {
  std::time_t end_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  std::cout << "start computation at " << std::ctime(&end_time) << std::endl;
  encoder_factory->add_dynamic_ = add_dynamic;
  encoder_factory->add_incremetal_ = add_incremental;
  simple_sat_solver::cardinality_benchmark::Test(test_file_path, test_file, log_dir , encoder_factory, encoder_string, start_penalty);
}


void test_file(std::string file, std::string simple_file, int start_penalty) {
  std::cout << "test file: " << file << std::endl;
  std::string log_dir = "../../../data/cardinality/logs3";
//  test_setting(file, simple_file, log_dir, (::Pumpkin::IEncoder<::Pumpkin::CardinalityConstraint>::IFactory *) new ::Pumpkin::IncrementalSequentialEncoder::Factory(), "Incremental", true, false, start_penalty);
//  test_setting(file, simple_file, log_dir, (::Pumpkin::IEncoder<::Pumpkin::CardinalityConstraint>::IFactory *) new ::Pumpkin::IncrementalSequentialEncoder::Factory(), "Incremental", true, true, start_penalty);
//  test_setting(file, simple_file, log_dir, (::Pumpkin::IEncoder<::Pumpkin::CardinalityConstraint>::IFactory *) new ::Pumpkin::PropagatorEncoder<Pumpkin::CardinalityConstraint>::Factory(), "Propagator", false, false, start_penalty);
//  test_setting(file, simple_file, log_dir, (::Pumpkin::IEncoder<::Pumpkin::CardinalityConstraint>::IFactory *) new ::Pumpkin::IncrementalSequentialEncoder::Factory(), "Incremental", false, false, start_penalty);

  test_setting(file, simple_file, log_dir, (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory *) new ::Pumpkin::GeneralizedTotaliser::Factory(), "Incremental", false, false, start_penalty);
//  test_setting(file, simple_file, log_dir, (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory *) new ::Pumpkin::PropagatorEncoder<Pumpkin::PseudoBooleanConstraint>::Factory(), "Propagator", false, false, start_penalty);
}

int main() {
  for (int j = 1; j <=10; ++j) {
    std::string x = std::to_string(1);
    std::string y = "d";
    for (int i = 2; i <= 20; i++) {
//      simple_sat_solver::cardinality_benchmark::TestFile(
  test_file(
          "/home/jens/Downloads/cc." + x + "/cnf." + std::to_string(i) + "." +
              y + "." + x,  "/cnf." + std::to_string(i) + "." + y + "." + x,
          200);
    }
//    y = "t";
//    for (int i = 2; i <= 20; i++) {
////      simple_sat_solver::cardinality_benchmark::TestFile(
//test_file(
//          "/home/jens/Downloads/cc." + x + "/cnf." + std::to_string(i) + "." +
//              y + "." + x, "/cnf." + std::to_string(i) + "." + y + "." + x,
//          200);
//    }
    y = "p";
    for (int i = 4; i <= 20; i++) {
//      simple_sat_solver::cardinality_benchmark::TestFile(

      test_file(
          "/home/jens/Downloads/cc." + x + "/cnf." + std::to_string(i) + "." +
              y + "." + x,"/cnf." + std::to_string(i) + "." + y + "." + x,
          200);
    }
  }
//  for (int i = 3; i <= 20; i++) {
//    simple_sat_solver::cardinality_benchmark::TestFile(
//        "/home/jens/Downloads/cc." + x +"/cnf." + std::to_string(i) + "." +y +"." + x, i-1);
//  }

}