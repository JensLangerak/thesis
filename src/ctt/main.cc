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
  void Test(std::string test_file_path, std::string test_file, std::string log_dir, ::Pumpkin::IEncoder::IFactory * encoder, std::string encoder_message, int start_penalty) {
    simple_sat_solver::logger::Logger::StartNewLog(log_dir,test_file);
    simple_sat_solver::logger::Logger::Log2("File: " + test_file_path);
    simple_sat_solver::logger::Logger::Log2("Encoder: " + encoder_message);
    std::string t = "T";
    std::string f = "F";
    simple_sat_solver::logger::Logger::Log2("Dynamic: " + (encoder->add_dynamic_ ? t : f));
    simple_sat_solver::logger::Logger::Log2("Incremental: " + (encoder->add_incremetal_ ? t : f));
    simple_sat_solver::logger::Logger::Log2("Delay factor: " + std::to_string(encoder->add_delay_));

    solver_wrappers::ISolver * solver = new solver_wrappers::Pumpkin(encoder, start_penalty);

    Ctt problem = Parser::Parse(test_file_path);
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

  void Test(std::string file, int start_penalty) {
    ::Pumpkin::IEncoder::IFactory * encoder_factory = new ::Pumpkin::IncrementalSequentialEncoder::Factory();
//    ::Pumpkin::IEncoder::IFactory * encoder_factory = new ::Pumpkin::TotaliserEncoder::Factory();
//    ::Pumpkin::IEncoder::IFactory * encoder_factory = new ::Pumpkin::PropagatorEncoder::Factory();
    encoder_factory->add_dynamic_ = true;
    encoder_factory->add_incremetal_ = true;
    solver_wrappers::ISolver * solver = new solver_wrappers::Pumpkin(encoder_factory, start_penalty);

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
void test_setting(std::string test_file_path, std::string test_file, std::string log_dir,
                  Pumpkin::IEncoder::IFactory *encoder_factory,
                  std::string encoder_string, bool add_dynamic,
                  bool add_incremental, int start_penalty, double add_delay) {
  std::time_t end_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  std::cout << "start computation at " << std::ctime(&end_time) << std::endl;
  encoder_factory->add_dynamic_ = add_dynamic;
  encoder_factory->add_incremetal_ = add_incremental;
  encoder_factory->add_delay_ = add_delay;
  simple_sat_solver::ctt::Test(test_file_path, test_file, log_dir , encoder_factory, encoder_string, start_penalty);
}

void test_file(std::string file, int start_penalty) {
  std::cout << "test file: " << file << std::endl;
  std::string dir = "../../../data/ctt/";
  std::string log_dir = dir + "log13";
  test_setting(dir + file, file, log_dir, (::Pumpkin::IEncoder::IFactory *) new ::Pumpkin::IncrementalSequentialEncoder::Factory(), "Incremental", true, false, start_penalty, 1);
  test_setting(dir + file, file, log_dir, (::Pumpkin::IEncoder::IFactory *) new ::Pumpkin::IncrementalSequentialEncoder::Factory(), "Incremental", true, true, start_penalty, 1);
  test_setting(dir + file, file, log_dir, (::Pumpkin::IEncoder::IFactory *) new ::Pumpkin::PropagatorEncoder::Factory(), "Propagator", false, false, start_penalty, 1);
  test_setting(dir + file, file, log_dir, (::Pumpkin::IEncoder::IFactory *) new ::Pumpkin::IncrementalSequentialEncoder::Factory(), "Incremental", false, false, start_penalty, 1);
}

enum solver_type {encoder, dynamic, incremental, propagator};
int main(int argc, char *argv[]) {
  if (argc >= 4) {
     solver_type s = (solver_type) atoi(argv[1]);
     std::string test_file = argv[2];
     std::string log_dir = argv[3];
     int add_delay_i = 10;
     if (argc >= 5) {
       add_delay_i = atoi(argv[4]);
     }
     double add_delay = ((double ) add_delay_i) / 10.0;
     int start_penalty = 600;
     ::Pumpkin::IEncoder::IFactory * factory;
     switch (s) {
     case encoder:
       test_setting(test_file, "test", log_dir, (::Pumpkin::IEncoder::IFactory *) new ::Pumpkin::IncrementalSequentialEncoder::Factory(), "Incremental", false, false, start_penalty, add_delay);
       break;
     case dynamic:
       test_setting(test_file, "test", log_dir, (::Pumpkin::IEncoder::IFactory *) new ::Pumpkin::IncrementalSequentialEncoder::Factory(), "Incremental", true, false, start_penalty, add_delay);
       break;
     case incremental:
       test_setting(test_file, "test", log_dir, (::Pumpkin::IEncoder::IFactory *) new ::Pumpkin::IncrementalSequentialEncoder::Factory(), "Incremental", true, true, start_penalty, add_delay);
       break;
     case propagator:
       test_setting(test_file, "test", log_dir, (::Pumpkin::IEncoder::IFactory *) new ::Pumpkin::PropagatorEncoder::Factory(), "Propagator", false, false, start_penalty, add_delay);
       break;
     default:
       return 1;
     }

  } else {

    //  simple_sat_solver::ctt::Test("../../../data/ctt/toyexample.ctt");
    // test_file("toyexample.ctt");
    //  test_file("comp01.ctt");
    //  test_file("comp02.ctt");
    //  test_file("comp03.ctt");
    //  test_file("comp04.ctt");
    //  test_file("comp05.ctt", 1000);
    //  test_file("comp06.ctt", 1000);
    test_file("comp07.ctt", 1000);
    //  test_file("comp12.ctt", 1000);

    //  ::Pumpkin::IEncoder::IFactory * encoder_factory = new ::Pumpkin::IncrementalSequentialEncoder::Factory();
    ////    ::Pumpkin::IEncoder::IFactory * encoder_factory = new ::Pumpkin::PropagatorEncoder::Factory();
    //  encoder_factory->add_dynamic_ = false;
    //  encoder_factory->add_incremetal_ = false;
    //  simple_sat_solver::ctt::Test("../../../data/ctt",file , encoder_factory, "Incremental");
  }
}
