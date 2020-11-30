//
// Created by jens on 11-11-20.
//
#include "../pumpkin/Propagators/Cardinality/Encoders/i_encoder.h"
#include "../pumpkin/Propagators/Cardinality/Encoders/incremental_sequential_encoder.h"
#include "../pumpkin/Propagators/Cardinality/Encoders/propagator_encoder.h"
#include "../pumpkin/Propagators/Cardinality/Encoders/totaliser_encoder.h"
#include "../solver_wrappers/i_solver.h"
#include "../solver_wrappers/pumpkin.h"
#include "ctt.h"
#include "ctt_converter.h"
#include "parser.h"
#include <ctime>
#include <iostream>
#include "../logger/logger.h"
void test_file(std::string file);
void test_setting(std::string file,
                  Pumpkin::IncrementalSequentialEncoder::Factory *encoder,
                  std::string encoder_string, bool add_dynamic,
                  bool add_incremental);
namespace simple_sat_solver::ctt {
  void Test(std::string dir, std::string file, ::Pumpkin::IEncoder::IFactory * encoder, std::string encoder_message) {
    simple_sat_solver::logger::Logger::StartNewLog(dir +"/logs3",file);
    simple_sat_solver::logger::Logger::Log2("File: " + file);
    simple_sat_solver::logger::Logger::Log2("Encoder: " + encoder_message);
    std::string t = "T";
    std::string f = "F";
    simple_sat_solver::logger::Logger::Log2("Dynamic: " + (encoder->add_dynamic_ ? t : f));
    simple_sat_solver::logger::Logger::Log2("Incremental: " + (encoder->add_incremetal_ ? t : f));

    solver_wrappers::ISolver * solver = new solver_wrappers::Pumpkin(encoder);

    Ctt problem = Parser::Parse(dir + "/" + file);
    CttConverter converter(problem);
    sat::SatProblem sat = converter.GetSatProblem();
    std::clock_t start = std::clock();
    bool res = solver->Optimize(sat);
    double duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
    std::cout << "time: " << duration <<std::endl;
    simple_sat_solver::logger::Logger::Log2("Time: " + std::to_string(duration));
//    assert(res);
    if (res) {
      auto sol = converter.ConvertSolution(solver->GetSolution());
//      converter.PrintSolution(sol);
//      std::ofstream outfile ("../../../data/ctt/solution.txt");
//      converter.PrintSolution(sol, outfile);
//      outfile.close();
      std::cout << "Penalty: " << converter.ValidateSolution(sol) << std::endl;
    }
    delete solver;
    simple_sat_solver::logger::Logger::End();
  }

  void Test(std::string file) {
    ::Pumpkin::IEncoder::IFactory * encoder_factory = new ::Pumpkin::IncrementalSequentialEncoder::Factory();
//    ::Pumpkin::IEncoder::IFactory * encoder_factory = new ::Pumpkin::TotaliserEncoder::Factory();
//    ::Pumpkin::IEncoder::IFactory * encoder_factory = new ::Pumpkin::PropagatorEncoder::Factory();
    encoder_factory->add_dynamic_ = true;
    encoder_factory->add_incremetal_ = true;
    solver_wrappers::ISolver * solver = new solver_wrappers::Pumpkin(encoder_factory);

    Ctt problem = Parser::Parse(file);
    CttConverter converter(problem);
    sat::SatProblem sat = converter.GetSatProblem();
//    bool res = solver->Optimize(sat);
    std::clock_t start = std::clock();
  bool res = solver->Optimize(sat);
    double duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
    std::cout << "time: " << duration <<std::endl;
//    assert(res);
    if (res) {
      auto sol = converter.ConvertSolution(solver->GetSolution());
      converter.PrintSolution(sol);
      std::ofstream outfile ("../../../data/ctt/solution.txt");
      converter.PrintSolution(sol, outfile);
      outfile.close();
      std::cout << "Penalty: " << converter.ValidateSolution(sol);
    }
    delete solver;
  }
}
void test_setting(std::string file,
                  Pumpkin::IEncoder::IFactory *encoder_factory,
                  std::string encoder_string, bool add_dynamic,
                  bool add_incremental) {
  std::time_t end_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  std::cout << "start computation at " << std::ctime(&end_time) << std::endl;
  encoder_factory->add_dynamic_ = add_dynamic;
  encoder_factory->add_incremetal_ = add_incremental;
  simple_sat_solver::ctt::Test("../../../data/ctt",file , encoder_factory, encoder_string);
}

void test_file(std::string file) {
  std::cout << "test file: " << file << std::endl;
//  test_setting(file,  (::Pumpkin::IEncoder::IFactory *) new ::Pumpkin::IncrementalSequentialEncoder::Factory(), "Incremental", false, false);
//  test_setting(file,  (::Pumpkin::IEncoder::IFactory *) new ::Pumpkin::IncrementalSequentialEncoder::Factory(), "Incremental", true, false);
//  test_setting(file,  (::Pumpkin::IEncoder::IFactory *) new ::Pumpkin::IncrementalSequentialEncoder::Factory(), "Incremental", true, true);
  test_setting(file,  (::Pumpkin::IEncoder::IFactory *) new ::Pumpkin::PropagatorEncoder::Factory(), "Propagator", false, false);
}

int main() {

//  simple_sat_solver::ctt::Test("../../../data/ctt/toyexample.ctt");
//test_file("toyexample.ctt");
//  test_file("comp01.ctt");
//  test_file("comp02.ctt");
//  test_file("comp03.ctt");
//  test_file("comp04.ctt");
  test_file("comp05.ctt");
  test_file("comp07.ctt");
  test_file("comp12.ctt");

//  ::Pumpkin::IEncoder::IFactory * encoder_factory = new ::Pumpkin::IncrementalSequentialEncoder::Factory();
////    ::Pumpkin::IEncoder::IFactory * encoder_factory = new ::Pumpkin::PropagatorEncoder::Factory();
//  encoder_factory->add_dynamic_ = false;
//  encoder_factory->add_incremetal_ = false;
//  simple_sat_solver::ctt::Test("../../../data/ctt",file , encoder_factory, "Incremental");
}
