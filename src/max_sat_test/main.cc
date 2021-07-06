//
// Created by jens on 15-12-20.
//
#include <ctime>
#include <filesystem>
#include <iostream>

#include "../logger/logger.h"
#include "../pumpkin/Utilities//solver_parameters.h"
#include "../pumpkin/Engine/constraint_optimisation_solver.h"
#include "../pumpkin/Propagators/Dynamic/Encoders/generalized_totaliser.h"
#include "../pumpkin/Propagators/Dynamic/Encoders/i_encoder.h"
#include "../pumpkin/Propagators/Dynamic/Encoders/propagator_encoder.h"
#include "../solver_wrappers/i_solver.h"
#include "../solver_wrappers/pumpkin.h"
#include "max_sat_bench_mark.h"

//void Test(std::string test_file_path, std::string test_file, std::string log_dir, ::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory * encoder, std::string encoder_message, int start_penalty) {
//  simple_sat_solver::logger::Logger::StartNewLog(log_dir,test_file);
//  simple_sat_solver::logger::Logger::Log2("File: " + test_file_path);
//  simple_sat_solver::logger::Logger::Log2("Encoder: " + encoder_message);
//  std::string t = "T";
//  std::string f = "F";
//  simple_sat_solver::logger::Logger::Log2("Dynamic: " + (encoder->add_dynamic_ ? t : f));
//  simple_sat_solver::logger::Logger::Log2("Incremental: " + (encoder->add_incremetal_ ? t : f));
//  simple_sat_solver::logger::Logger::Log2("Delay factor: " + std::to_string(encoder->add_delay_));
//
//  std::cout << "Reading file " << test_file_path << std::endl;
//  Pumpkin::ProblemSpecification *problem = new Pumpkin::ProblemSpecification(Pumpkin::ProblemSpecification::ReadMaxSATFormula(test_file_path));
//  std::cout << "Done reading" << std::endl;
//  Pumpkin::SolverParameters params;
//  params.bump_decision_variables = true;
//  Pumpkin::ConstraintOptimisationSolver solver(*problem, params);
//  solver.optimisation_encoding_factory = encoder; // TODO
////  SolverOutput solver_output = solver.Solve(std::numeric_limits<double>::max());
//  solver.start_upper_bound_ = 100000;
//  Pumpkin::SolverOutput solver_output = solver.Solve(300);
//  bool solved = solver_output.HasSolution();
//
//  if (solved)
//    std::cout << "Solution found" << std::endl;
//
//  simple_sat_solver::logger::Logger::End();
//  delete problem;
//}



void TestOrder(std::string test_file_path, std::string test_file, std::string log_dir, int start_penalty) {
//  simple_sat_solver::logger::Logger::StartNewLog(log_dir,test_file);
//  simple_sat_solver::logger::Logger::Log2("File: " + test_file_path);
//  simple_sat_solver::logger::Logger::Log2("Encoder: Encoder_weight10");
//  std::string t = "T";
//  std::string f = "F";
//  simple_sat_solver::logger::Logger::Log2("Dynamic: F" );
//  simple_sat_solver::logger::Logger::Log2("Incremental: F");
//  simple_sat_solver::logger::Logger::Log2("Delay factor: 0");
//  auto encoder_factory = (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory
//  *)new ::Pumpkin::GeneralizedTotaliser::Factory();
//  encoder_factory->add_dynamic_ = false;
//  encoder_factory->add_incremetal_ = false;
//  encoder_factory->add_delay_ = 0;
//
//  std::cout << "Reading file " << test_file_path << std::endl;
//  Pumpkin::ProblemSpecification *problem = new Pumpkin::ProblemSpecification(Pumpkin::ProblemSpecification::ReadMaxSATFormula(test_file_path));
//  for (int i = 0; i < problem->objective_literals_.size(); ++i) {
//    problem->objective_literals_[i].weight = 1;
//  }
//  std::cout << "num lits " << std::to_string(problem->objective_literals_.size()) << std::endl;
//
//  std::sort(problem->objective_literals_.begin(), problem->objective_literals_.end(),
//            [](::Pumpkin::WeightedLiteral a, Pumpkin::WeightedLiteral b) {
//              return a.weight > b.weight;
//            });
//  std::cout << "Done reading" << std::endl;
//  Pumpkin::SolverParameters params;
//  params.bump_decision_variables = true;
//  Pumpkin::ConstraintOptimisationSolver solver(*problem, params);
//  solver.optimisation_encoding_factory = encoder_factory; // TODO
////  SolverOutput solver_output = solver.Solve(std::numeric_limits<double>::max());
//  solver.start_upper_bound_ = 1000;
//  Pumpkin::SolverOutput solver_output = solver.Solve(10);
//  bool solved = solver_output.HasSolution();
//
//  if (solved) {
//    std::cout << "Solution found" << std::endl;
//  }
//  simple_sat_solver::logger::Logger::StartNewLog(log_dir,test_file);
//  simple_sat_solver::logger::Logger::Log2("File: " + test_file_path);
//  simple_sat_solver::logger::Logger::Log2("Encoder: Encoder_activity10");
//  simple_sat_solver::logger::Logger::Log2("Dynamic: F" );
//  simple_sat_solver::logger::Logger::Log2("Incremental: F");
//  simple_sat_solver::logger::Logger::Log2("Delay factor: 0");
//
//  Pumpkin::ProblemSpecification *problem2 = new Pumpkin::ProblemSpecification(Pumpkin::ProblemSpecification::ReadMaxSATFormula(test_file_path));
//  for (int i = 0; i < problem2->objective_literals_.size(); ++i) {
//    problem2->objective_literals_[i].weight = 1;
//  }
//  std::sort(problem2->objective_literals_.begin(), problem2->objective_literals_.end(),
//            [&solver](::Pumpkin::WeightedLiteral a, Pumpkin::WeightedLiteral b) {
//              if (a.weight == b.weight) {
//          int a_act = solver.constrained_satisfaction_solver_.state_
//                          .variable_selector_.heap_.GetKeyValue(a.literal.VariableIndex() - 1);
//          int b_act = solver.constrained_satisfaction_solver_.state_
//                          .variable_selector_.heap_.GetKeyValue(b.literal.VariableIndex() - 1);
//          return a_act > b_act;
//        }
//              return a.weight > b.weight;
//            });
//
//  Pumpkin::ConstraintOptimisationSolver solver2(*problem2, params);
//  solver2.optimisation_encoding_factory = encoder_factory; // TODO
////  SolverOutput solver_output = solver.Solve(std::numeric_limits<double>::max());
//  solver2.start_upper_bound_ = 1000;
//  Pumpkin::SolverOutput solver_output2 = solver2.Solve(600);
//  bool solved2 = solver_output.HasSolution();
//
//  if (solved2) {
//    std::cout << "Solution found" << std::endl;
//  }
//
//  simple_sat_solver::logger::Logger::End();
//  delete problem;
//  delete problem2;
}
//
//
//void test_setting(std::string test_file_path, std::string test_file, std::string log_dir,
//                  Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory *encoder_factory,
//                  std::string encoder_string, bool add_dynamic,
//                  bool add_incremental, int start_penalty, double add_delay) {
//  std::time_t end_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
//
//  std::cout << "start computation at " << std::ctime(&end_time) << std::endl;
//  encoder_factory->add_dynamic_ = add_dynamic;
//  encoder_factory->add_incremetal_ = add_incremental;
//  encoder_factory->add_delay_ = add_delay;
////  TestOrder(test_file_path, test_file, log_dir , start_penalty);
//  Test(test_file_path, test_file, log_dir , encoder_factory, encoder_string, start_penalty);
//}



using namespace simple_sat_solver::max_sat;
using namespace simple_sat_solver::benchmark;
int main(int argc, char *argv[]) {
  MaxSatBenchMark benchnark;
  if (argc >= 4) {
    benchnark.Main(argc, argv);
  } else {
    //  std::string test_file = "/home/jens/CLionProjects/SimpleSatSolver/data/max_sat/pima_train_3_CNF_5_20.wcnf" ;//"../../../data/max_sat/cnf_small.wcnf";
//  std::string test_file = "/home/jens/CLionProjects/SimpleSatSolver/data/max_sat/d4.wcnf" ;//"../../../data/max_sat/cnf_small.wcnf";
//  std::string test_file = "/home/jens/Downloads/maxsat_instances (2)/ms_evals/MS20/mse20-incomplete-weighted/af-synthesis/af-synthesis_stb_50_140_8.wcnf" ;//"../../../data/max_sat/cnf_small.wcnf";
//    std::string test_file = "/home/jens/starexec_files/max_sat_incomplete_w1/causal_Autos_8_159.wcnf";
    std::string test_file = "/home/jens/starexec_files/max_sat_incomplete_w1/causal_carpo_9_10000.wcnf";
//    std::string test_file = "/home/jens/starexec_files/max_sat_incomplete_w1/Rounded_BTWBNSL_hailfinder_10000_1_3.scores_TWBound_3.wcnf";
//  std::string test_file = "/home/jens/Downloads/maxsat_instances (2)/ms_evals/MS20/mse20-incomplete-weighted/min-width/MinWidthCB_mitdbsample_300_32_1k_6s_1t_8.wcnf";
//    std::string test_file = "/home/jens/Downloads/maxsat_instances (2)/ms_evals/MS20/mse20-incomplete-weighted/MaxSATQueriesinInterpretableClassifiers/titanic_train_4_DNF_4_20.wcnf";
    std::string log_dir="/home/jens/CLionProjects/SimpleSatSolver/data/max_sat/log2";// "../../../data/max_sat/log/";
    std::string test_dir = "/home/jens/CLionProjects/SimpleSatSolver/data/max_sat/uw/";
//  for (const auto & entry : std::filesystem::directory_iterator(test_dir)) {
//    TestOrder(entry.path().generic_string(), entry.path().filename().generic_string(), log_dir, 1000);
//  }


    benchnark.solver_type_ = simple_sat_solver::benchmark::SolverType::ENCODER;
    benchnark.problem_file_full_path_ =test_file;
    benchnark.log_dir_ = log_dir;
    benchnark.delay_factor_ = 0.1;
    benchnark.start_penalty_ = 100000;

    benchnark.Main();
  }


//
////  return 0;
//  solver_type s = incremental;
//  int add_delay_i = 0;
//  if (argc >= 4) {
//    s = (solver_type)atoi(argv[1]);
//    test_file = argv[2];
//    log_dir = argv[3];
//    if (argc >= 5) {
//      add_delay_i = atoi(argv[4]);
//    }
//  }
//
//    double add_delay = ((double)add_delay_i) / 10.0;
//    int start_penalty = 600;
//    ::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory *factory;
//    switch (s) {
//    case encoder:
//      test_setting(
//          test_file, "test", log_dir,
//          (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory
//               *)new ::Pumpkin::GeneralizedTotaliser::Factory(),
//          "Incremental", false, false, start_penalty, add_delay);
//      break;
//    case dynamic:
//      test_setting(
//          test_file, "test", log_dir,
//          (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory
//               *)new ::Pumpkin::GeneralizedTotaliser::Factory(),
//          "Incremental", true, false, start_penalty, add_delay);
//      break;
//    case incremental:
//      test_setting(
//          test_file, "test", log_dir,
//          (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory
//               *)new ::Pumpkin::GeneralizedTotaliser::Factory(),
//          "Incremental", true, true, start_penalty, add_delay);
//      break;
//    case propagator:
//      test_setting(
//          test_file, "test", log_dir,
//          (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory
//               *)new ::Pumpkin::
//              PropagatorEncoder<Pumpkin::PseudoBooleanConstraint>::Factory(),
//          "Propagator", false, false, start_penalty, add_delay);
//      break;
//    case static_incremental:
//      test_setting(
//          test_file, "test", log_dir,
//          (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory
//          *)new ::Pumpkin::StaticGeneralizedTotaliser::Factory(),
//          "Static", true, true, start_penalty, add_delay);
//      break;
//    case sum:
//      test_setting(
//          test_file, "test", log_dir,
//          (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory
//          *)new ::Pumpkin::GeneralizedTotliserSumRoot::Factory(),
//          "SumPB", true, true, start_penalty, add_delay);
//      break;
//    default:
//      return 1;
//  }
}