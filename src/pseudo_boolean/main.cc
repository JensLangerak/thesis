//
// Created by jens on 15-12-20.
//
#include <ctime>
#include <iostream>

#include "../logger/logger.h"
#include "../pumpkin/Basic Data Structures/solver_parameters.h"
#include "../pumpkin/Engine/constraint_optimisation_solver.h"
#include "../pumpkin/Propagators/Dynamic/Encoders/generalized_totaliser.h"
#include "../pumpkin/Propagators/Dynamic/Encoders/i_encoder.h"
#include "../pumpkin/Propagators/Dynamic/Encoders/incremental_sequential_encoder.h"
#include "../pumpkin/Propagators/Dynamic/Encoders/propagator_encoder.h"
#include "../pumpkin/Propagators/Dynamic/Encoders/totaliser_encoder.h"
#include "../solver_wrappers/i_solver.h"
#include "../solver_wrappers/pumpkin.h"
#include "../sat/sat_problem.h"
#include "opb_parser.h"

void Test(std::string test_file_path, std::string test_file, std::string log_dir, ::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory * encoder, std::string encoder_message, int start_penalty) {
  simple_sat_solver::logger::Logger::StartNewLog(log_dir,test_file);
  simple_sat_solver::logger::Logger::Log2("File: " + test_file_path);
  simple_sat_solver::logger::Logger::Log2("Encoder: " + encoder_message);
  std::string t = "T";
  std::string f = "F";
  simple_sat_solver::logger::Logger::Log2("Dynamic: " + (encoder->add_dynamic_ ? t : f));
  simple_sat_solver::logger::Logger::Log2("Incremental: " + (encoder->add_incremetal_ ? t : f));
  simple_sat_solver::logger::Logger::Log2("Delay factor: " + std::to_string(encoder->add_delay_));

  std::cout << "Reading file " << test_file_path << std::endl;
  simple_sat_solver::sat::SatProblem problem = simple_sat_solver::pseudo_boolean::OpbParser::Parse(test_file_path);
  std::cout << "Done reading" << std::endl;

  simple_sat_solver::solver_wrappers::ISolver * solver = new simple_sat_solver::solver_wrappers::Pumpkin(encoder, encoder, start_penalty);

  std::clock_t start = std::clock();
//  bool res = solver->Optimize(problem);
  bool res = solver->Solve(problem);
  double duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  std::cout << "time: " << duration <<std::endl;
  simple_sat_solver::logger::Logger::Log2("Time: " + std::to_string(duration));
//    assert(res);
  if (res) {
    std::cout << "Solution found" << std::endl;
  } else {
    std::cout << "No solution found " << std::endl;
  }
  delete solver;
  simple_sat_solver::logger::Logger::End();

}


void test_setting(std::string test_file_path, std::string test_file, std::string log_dir,
                  Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory *encoder_factory,
                  std::string encoder_string, bool add_dynamic,
                  bool add_incremental, int start_penalty, double add_delay) {
  std::time_t end_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  std::cout << "start computation at " << std::ctime(&end_time) << std::endl;
  encoder_factory->add_dynamic_ = add_dynamic;
  encoder_factory->add_incremetal_ = add_incremental;
  encoder_factory->add_delay_ = add_delay;
  Test(test_file_path, test_file, log_dir , encoder_factory, encoder_string, start_penalty);
}


enum solver_type {encoder, dynamic, incremental, propagator};
int main(int argc, char *argv[]) {
//  std::string test_file = "/home/jens/Downloads/PB10/normalized-PB10/DEC-SMALLINT-LIN/leberre/opb-paranoid/misc2010/datasets/caixa/normalized-1096.cudf.paranoid.opb";
  std::string test_file = "/home/jens/Downloads/PB10/normalized-PB10/DEC-SMALLINT-LIN/oliveras/j30/normalized-j3014_7-sat.opb";
//  std::string test_file = "/home/jens/Downloads/PB10/normalized-PB10/DEC-SMALLINT-LIN/oliveras/j120/normalized-j12012_9-unsat.opb";
//  std::string test_file = "/home/jens/Downloads/normalized-PB09/OPT-SMALLINT-LIN/flexray/normalized-fx30.opb";
  std::string log_dir="/home/jens/CLionProjects/SimpleSatSolver/data/pb/log";
  solver_type s = propagator;
  int add_delay_i = 0;
  if (argc >= 4) {
    s = (solver_type)atoi(argv[1]);
    test_file = argv[2];
    log_dir = argv[3];
    if (argc >= 5) {
      add_delay_i = atoi(argv[4]);
    }
  }

    double add_delay = ((double)add_delay_i) / 10.0;
    int start_penalty = 600000;
    ::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory *factory;
    switch (s) {
    case encoder:
      test_setting(
          test_file, "test", log_dir,
          (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory
               *)new ::Pumpkin::GeneralizedTotaliser::Factory(),
          "Incremental", false, false, start_penalty, add_delay);
      break;
    case dynamic:
      test_setting(
          test_file, "test", log_dir,
          (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory
               *)new ::Pumpkin::GeneralizedTotaliser::Factory(),
          "Incremental", true, false, start_penalty, add_delay);
      break;
    case incremental:
      test_setting(
          test_file, "test", log_dir,
          (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory
               *)new ::Pumpkin::GeneralizedTotaliser::Factory(),
          "Incremental", true, true, start_penalty, add_delay);
      break;
    case propagator:
      test_setting(
          test_file, "test", log_dir,
          (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory
               *)new ::Pumpkin::
              PropagatorEncoder<Pumpkin::PseudoBooleanConstraint>::Factory(),
          "Propagator", false, false, start_penalty, add_delay);
      break;
    default:
      return 1;
  }
}