//
// Created by jens on 11-11-20.
//
//#include "../logger/logger.h"
//#include "../pumpkin/Basic Data Structures/solver_parameters.h"
//#include "../pumpkin/Engine/constraint_optimisation_solver.h"
//#include "../pumpkin/Propagators/Dynamic/Encoders/generalized_totaliser.h"
//#include "../pumpkin/Propagators/Dynamic/Encoders/generalized_totliser_sum_root.h"
//#include "../pumpkin/Propagators/Dynamic/Encoders/i_encoder.h"
//#include "../pumpkin/Propagators/Dynamic/Encoders/incremental_sequential_encoder.h"
//#include "../pumpkin/Propagators/Dynamic/Encoders/propagator_encoder.h"
//#include "../pumpkin/Propagators/Dynamic/Encoders/static_generalized_totaliser.h"
//#include "../pumpkin/Propagators/Dynamic/Encoders/totaliser_encoder.h"
//#include "../sat/constraints/cardinality_constraint.h"
//#include "../sat/constraints/pseudo_boolean_constraint.h"
//#include "../sat/constraints/sum_constraint.h"
//#include "../solver_wrappers/i_solver.h"
//#include "../solver_wrappers/pumpkin.h"
#include "../benchmark/benchmark.h"
#include "ctt.h"
#include "ctt_benchmark.h"
#include "ctt_converter.h"
#include "parser.h"
#include <algorithm>
#include <ctime>
#include <iostream>
// void test_file(std::string file);
//void test_setting(std::string file,
//                  Pumpkin::IncrementalSequentialEncoder::Factory *encoder,
//                  std::string encoder_string, bool add_dynamic,
//                  bool add_incremental);
//namespace simple_sat_solver::ctt {
//Pumpkin::ProblemSpecification
//ConvertProblem(SatProblem p,
//               ::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory
//                   *constraint_encoder);
//void TestOrder(
//    std::string test_file_path, std::string test_file, std::string log_dir,
//    ::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory *encoder,
//    std::string encoder_message, int start_penalty) {
//  Ctt ctt_problem = Parser::Parse(test_file_path);
//  CttConverter converter(ctt_problem);
//  sat::SatProblem sat_original = converter.GetSatProblem();
//
//  sat::SatProblem sat_weight(sat_original.GetNrVars());
//  {
//    for (auto c : sat_original.GetClauses()) {
//      sat_weight.AddClause(c);
//    }
//    auto minimize = sat_original.GetMinimizeLit();
//    std::sort(minimize.begin(), minimize.end(),
//              [](WeightedLit a, WeightedLit b) {
//                return a.w > b.w;
//              });
//    for (auto l : minimize)
//      sat_weight.AddToMinimize(l);
//
//    for (auto c : sat_original.GetConstraints()) {
//      if (PseudoBooleanConstraint *pb =
//              dynamic_cast<PseudoBooleanConstraint *>(c)) {
//        auto lits = pb->weighted_lits_;
//        std::sort(lits.begin(), lits.end(),
//                  [](WeightedLit a, WeightedLit b) {
//                    return a.w > b.w;
//                  });
//        std::vector<Lit> literals;
//        std::vector<int> weights;
//        for (WeightedLit wl : lits) {
//          literals.push_back(wl.l);
//          weights.push_back(wl.w);
//        }
//        PseudoBooleanConstraint *pb_new = new PseudoBooleanConstraint(pb);
//            new PseudoBooleanConstraint(literals, weights, pb->min_, pb->max_);
////        sat_weight.AddConstraint(pb_new);
//      } else   if (CardinalityConstraint *pb =
//          dynamic_cast<CardinalityConstraint *>(c)) {
//        auto lits = pb->lits;
//        CardinalityConstraint *pb_new =
////            new CardinalityConstraint(*pb);
//            new CardinalityConstraint(lits, pb->min, pb->max);
//        sat_weight.AddConstraint(pb_new);
//      }
//
//
//      else {
//
//        assert(false); // TODO
//      }
//    }
//  }
//  simple_sat_solver::logger::Logger::StartNewLog(log_dir, test_file);
//  simple_sat_solver::logger::Logger::Log2("V 2");
//  simple_sat_solver::logger::Logger::Log2("File: " + test_file_path);
//  simple_sat_solver::logger::Logger::Log2("Encoder: Encoder_weight");
//  simple_sat_solver::logger::Logger::Log2("ConstraintEncoder: Encoder");
//  std::string t = "T";
//  std::string f = "F";
//  simple_sat_solver::logger::Logger::Log2("Dynamic: F");
//  simple_sat_solver::logger::Logger::Log2("Incremental: F ");
//  simple_sat_solver::logger::Logger::Log2("Delay factor: " +
//                                          std::to_string(encoder->add_delay_));
//  auto constraint_encoder =
//      (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory
//           *)new ::Pumpkin::GeneralizedTotaliser::Factory();
//  constraint_encoder->add_dynamic_ = false;
//  constraint_encoder->add_incremetal_ = false;
//  //    auto constraint_encoder = encoder;
//  {
////    solver_wrappers::ISolver *solver =
////        new solver_wrappers::Pumpkin(constraint_encoder, start_penalty);
////
////    bool res = solver->Optimize(sat_weight);
////    if (res) {
////      auto sol = converter.ConvertSolution(solver->GetSolution());
////      std::cout << "Penalty: " << converter.ValidateSolution(sol) << std::endl;
////    }
//  }
//
//
//  Pumpkin::SolverParameters params;
//  params.bump_decision_variables = true;
//
//  /// Test the weight order
//  Pumpkin::ProblemSpecification problem =
//      ConvertProblem(sat_original, constraint_encoder);
//  Pumpkin::ConstraintOptimisationSolver solver(problem, params);
//  solver.optimisation_encoding_factory = constraint_encoder; // TODO
//  solver.start_upper_bound_ = 100000;
//  std::clock_t start = std::clock();
//  ::Pumpkin::SolverOutput solver_output = solver.Solve(10);
//  double duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;
//  std::cout << "time: " << duration << std::endl;
//  simple_sat_solver::logger::Logger::Log2("Time: " + std::to_string(duration));
//  bool res = solver_output.HasSolution();
//  if (res) {
//    auto solution = std::vector<bool>();
//    for (int i = 1; i < solver_output.solution.size(); ++i)
//      solution.push_back(solver_output.solution[i]);
//    auto sol = converter.ConvertSolution(solution);
//    std::cout << "Penalty: " << converter.ValidateSolution(sol) << std::endl;
//  }
//  simple_sat_solver::logger::Logger::End();
//
//  /// Test activiy order
//  sat::SatProblem sat_activity(sat_original.GetNrVars());
//  {
//    for (auto c : sat_original.GetClauses()) {
//      sat_activity.AddClause(c);
//    }
//    auto minimize = sat_original.GetMinimizeLit();
//    std::sort(minimize.begin(), minimize.end(),
//              [&solver](WeightedLit a, WeightedLit b) {
//                if (a.w == b.w) {
//                  int a_act = solver.constrained_satisfaction_solver_.state_
//                                  .variable_selector_.heap_.GetKeyValue(
//                                      a.l.x);
//                  int b_act = solver.constrained_satisfaction_solver_.state_
//                                  .variable_selector_.heap_.GetKeyValue(
//                                      b.l.x);
//                  return a_act > b_act;
//                } else {
//                  return a.w > b.w;
//                }
//              });
//    for (auto l : minimize)
//      sat_activity.AddToMinimize(l);
//
//    for (auto c : sat_original.GetConstraints()) {
//      if (PseudoBooleanConstraint *pb =
//              dynamic_cast<PseudoBooleanConstraint *>(c)) {
//        auto lits = pb->weighted_lits_;
//        std::sort(
//            lits.begin(), lits.end(), [&solver](WeightedLit a, WeightedLit b) {
//              if (a.w == b.w) {
//                int a_act = solver.constrained_satisfaction_solver_.state_
//                                .variable_selector_.heap_.GetKeyValue(a.l.x);
//                int b_act = solver.constrained_satisfaction_solver_.state_
//                                .variable_selector_.heap_.GetKeyValue(b.l.x);
//                return a_act > b_act;
//              } else {
//                return a.w > b.w;
//              }
//            });
//        std::vector<Lit> literals;
//        std::vector<int> weights;
//        for (WeightedLit wl : lits) {
//          literals.push_back(wl.l);
//          weights.push_back(wl.w);
//        }
//        PseudoBooleanConstraint *pb_new =
//            new PseudoBooleanConstraint(literals, weights, pb->min_, pb->max_);
//        sat_activity.AddConstraint(pb_new);
//      } else if (CardinalityConstraint *pb =
//          dynamic_cast<CardinalityConstraint *>(c)) {
//        auto lits = pb->lits;
//        std::sort(
//            lits.begin(), lits.end(),
//            [&solver](Lit a, Lit b) {
//                int a_act = solver.constrained_satisfaction_solver_.state_
//                    .variable_selector_.heap_.GetKeyValue(
//                    a.x );
//                int b_act = solver.constrained_satisfaction_solver_.state_
//                    .variable_selector_.heap_.GetKeyValue(
//                    b.x);
//                return a_act > b_act;
//            });
//
//        CardinalityConstraint *pb_new =
//            new CardinalityConstraint(lits, pb->min, pb->max);
//        sat_activity.AddConstraint(pb_new);
//      } else {
//        assert(false); // TODO
//      }
//    }
//  }
//
//  simple_sat_solver::logger::Logger::StartNewLog(log_dir, test_file);
//  simple_sat_solver::logger::Logger::Log2("V 2");
//  simple_sat_solver::logger::Logger::Log2("File: " + test_file_path);
//  simple_sat_solver::logger::Logger::Log2("Encoder: Encoder_activity");
//  simple_sat_solver::logger::Logger::Log2("ConstraintEncoder: Encoder");
//  simple_sat_solver::logger::Logger::Log2("Dynamic: F");
//  simple_sat_solver::logger::Logger::Log2("Incremental: F ");
//  simple_sat_solver::logger::Logger::Log2("Delay factor: " +
//                                          std::to_string(encoder->add_delay_));
//
//  Pumpkin::ProblemSpecification problem2 =
//      ConvertProblem(sat_activity, constraint_encoder);
//  Pumpkin::ConstraintOptimisationSolver solver2(problem2, params);
//  solver2.optimisation_encoding_factory = constraint_encoder; // TODO
//  solver2.start_upper_bound_ = 100000;
//  start = std::clock();
//  ::Pumpkin::SolverOutput solver_output2 = solver2.Solve(900);
//  duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;
//  std::cout << "time: " << duration << std::endl;
//  simple_sat_solver::logger::Logger::Log2("Time: " + std::to_string(duration));
//  bool res2 = solver_output.HasSolution();
//  if (res) {
//    auto solution = std::vector<bool>();
//    for (int i = 1; i < solver_output2.solution.size(); ++i)
//      solution.push_back(solver_output2.solution[i]);
//    auto sol = converter.ConvertSolution(solution);
//    std::cout << "Penalty: " << converter.ValidateSolution(sol) << std::endl;
//  }
//  simple_sat_solver::logger::Logger::End();
//}
//Pumpkin::ProblemSpecification
//ConvertProblem(SatProblem p,
//               Pumpkin::IEncoder<Pumpkin::PseudoBooleanConstraint>::IFactory
//                   *constraint_encoder) {
//  Pumpkin::ProblemSpecification problem;
//  problem.num_Boolean_variables_ = p.GetNrVars();
//  for (const auto &c : p.GetClauses()) {
//    std::vector<::Pumpkin::BooleanLiteral> clause;
//    for (sat::Lit l : c) {
//      ::Pumpkin::BooleanLiteral lit = ::Pumpkin::BooleanLiteral(
//          ::Pumpkin::BooleanVariable(l.x + 1), !l.complement);
//      clause.push_back(lit);
//    }
//    problem.AddClause(clause);
//  }
//
//  for (sat::IConstraint *c : p.GetConstraints()) {
//    if (sat::CardinalityConstraint *car =
//            dynamic_cast<sat::CardinalityConstraint *>(c)) {
//      std::vector<::Pumpkin::BooleanLiteral> lits;
//      std::vector<uint32_t> weights;
//      for (sat::Lit l : car->lits) {
//        ::Pumpkin::BooleanLiteral lit = ::Pumpkin::BooleanLiteral(
//            ::Pumpkin::BooleanVariable(l.x + 1), !l.complement);
//        lits.push_back(lit);
//        weights.push_back(1);
//      }
//      assert(car->min == 0);
//      problem.pseudo_boolean_constraints_.push_back(
//          ::Pumpkin::PseudoBooleanConstraint(lits, weights, car->max,
//                                             constraint_encoder));
//      //    }
//    } else if (sat::SumConstraint *car =
//                   dynamic_cast<sat::SumConstraint *>(c)) {
//      std::vector<::Pumpkin::BooleanLiteral> inputs;
//      for (sat::Lit l : car->input_lits_) {
//        ::Pumpkin::BooleanLiteral lit = ::Pumpkin::BooleanLiteral(
//            ::Pumpkin::BooleanVariable(l.x + 1), !l.complement);
//        inputs.push_back(lit);
//      }
//      std::vector<::Pumpkin::BooleanLiteral> outputs;
//      for (sat::Lit l : car->output_lits_) {
//        ::Pumpkin::BooleanLiteral lit = ::Pumpkin::BooleanLiteral(
//            ::Pumpkin::BooleanVariable(l.x + 1), !l.complement);
//        outputs.push_back(lit);
//      }
//
//      assert(false);
//      // TODO
//      //      problem.sum_constraints_.push_back(
//      //          ::Pumpkin::SumConstraint(inputs, outputs, encoder_factory_));
//    } else if (sat::PseudoBooleanConstraint *car =
//                   dynamic_cast<sat::PseudoBooleanConstraint *>(c)) {
//      std::vector<::Pumpkin::BooleanLiteral> lits;
//      std::vector<uint32_t> weights;
//      for (sat::WeightedLit l : car->weighted_lits_) {
//        ::Pumpkin::BooleanLiteral lit = ::Pumpkin::BooleanLiteral(
//            ::Pumpkin::BooleanVariable(l.l.x + 1), !l.l.complement);
//        lits.push_back(lit);
//        weights.push_back(l.w);
//      }
//
//      problem.pseudo_boolean_constraints_.push_back(
//          ::Pumpkin::PseudoBooleanConstraint(lits, weights, car->max_,
//                                             constraint_encoder));
//
//    } else {
//      assert(false);
//    }
//  }
//  // TODO
//  auto t = p.GetMinimizeLit();
//  for (auto l : p.GetMinimizeLit()) {
//    problem.objective_literals_.push_back(Pumpkin::WeightedLiteral(
//        ::Pumpkin::BooleanLiteral(Pumpkin::BooleanVariable(l.l.x + 1), true),
//        l.w));
//  }
//  return problem;
//}
//void Test(
//    std::string test_file_path, std::string test_file, std::string log_dir,
//    ::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory *encoder,
//    std::string encoder_message, int start_penalty) {
//  simple_sat_solver::logger::Logger::StartNewLog(log_dir, test_file);
//  simple_sat_solver::logger::Logger::Log2("V 2");
//  simple_sat_solver::logger::Logger::Log2("File: " + test_file_path);
//  simple_sat_solver::logger::Logger::Log2("Encoder: " + encoder_message);
//  simple_sat_solver::logger::Logger::Log2("ConstraintEncoder: Propagator");
//  std::string t = "T";
//  std::string f = "F";
//  simple_sat_solver::logger::Logger::Log2("Dynamic: " +
//                                          (encoder->add_dynamic_ ? t : f));
//  simple_sat_solver::logger::Logger::Log2("Incremental: " +
//                                          (encoder->add_incremetal_ ? t : f));
//  simple_sat_solver::logger::Logger::Log2("Delay factor: " +
//                                          std::to_string(encoder->add_delay_));
//  auto constraint_encoder =
//      (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory
//           *)new ::Pumpkin::
//          PropagatorEncoder<Pumpkin::PseudoBooleanConstraint>::Factory();
//  constraint_encoder->add_dynamic_ = false;
//  constraint_encoder->add_incremetal_ = false;
//  //    auto constraint_encoder = encoder;
//  solver_wrappers::ISolver *solver =
//      new solver_wrappers::Pumpkin(encoder, constraint_encoder, start_penalty);
//
//  Ctt problem = Parser::Parse(test_file_path);
//  CttConverter converter(problem);
//  sat::SatProblem sat = converter.GetSatProblem();
//  std::clock_t start = std::clock();
//  bool res = solver->Optimize(sat);
//  double duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;
//  std::cout << "time: " << duration << std::endl;
//  simple_sat_solver::logger::Logger::Log2("Time: " + std::to_string(duration));
//  //    assert(res);
//  if (res) {
//    auto sol = converter.ConvertSolution(solver->GetSolution());
//    //      converter.PrintSolution(sol);
//    std::ofstream outfile(log_dir + "/solution.txt");
//    converter.PrintSolution(sol, outfile);
//    outfile.flush();
//    outfile.close();
//    std::cout << "Penalty: " << converter.ValidateSolution(sol) << std::endl;
//  }
//  delete solver;
//  simple_sat_solver::logger::Logger::End();
//}
//
//void Test(std::string file, int start_penalty) {
//  ::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory
//      *encoder_factory = new ::Pumpkin::GeneralizedTotaliser::Factory();
//  //    ::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory *
//  //    encoder_factory = new ::Pumpkin::TotaliserEncoder::Factory();
//  //    ::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory *
//  //    encoder_factory = new ::Pumpkin::PropagatorEncoder::Factory();
//  encoder_factory->add_dynamic_ = true;
//  encoder_factory->add_incremetal_ = true;
//  solver_wrappers::ISolver *solver =
//      new solver_wrappers::Pumpkin(encoder_factory, start_penalty);
//
//  Ctt problem = Parser::Parse(file);
//  CttConverter converter(problem);
//  sat::SatProblem sat = converter.GetSatProblem();
//  //    bool res = solver->Optimize(sat);
//  std::clock_t start = std::clock();
//  bool res = solver->Optimize(sat);
//  double duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;
//  std::cout << "time: " << duration << std::endl;
//  //    assert(res);
//  if (res) {
//    auto sol = converter.ConvertSolution(solver->GetSolution());
//    converter.PrintSolution(sol);
//    std::ofstream outfile("../../../data/ctt/solution.txt");
//    converter.PrintSolution(sol, outfile);
//    outfile.close();
//    std::cout << "Penalty: " << converter.ValidateSolution(sol);
//  }
//  delete solver;
//}
//} // namespace simple_sat_solver::ctt
//void test_setting(
//    std::string test_file_path, std::string test_file, std::string log_dir,
//    Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory
//        *encoder_factory,
//    std::string encoder_string, bool add_dynamic, bool add_incremental,
//    int start_penalty, double add_delay) {
//  std::time_t end_time =
//      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
//
//  std::cout << "start computation at " << std::ctime(&end_time) << std::endl;
//  encoder_factory->add_dynamic_ = add_dynamic;
//  encoder_factory->add_incremetal_ = add_incremental;
//  encoder_factory->add_delay_ = add_delay;
//  simple_sat_solver::ctt::Test(test_file_path, test_file, log_dir,
//                               encoder_factory, encoder_string, start_penalty);
//}

//void test_file(std::string file, int start_penalty) {
//  std::cout << "test file: " << file << std::endl;
//  std::string dir = "../../../data/ctt/";
//  std::string log_dir = dir + "logs_a4";
////    test_setting(
////        dir + file, file, log_dir,
////        (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory
////             *)new ::Pumpkin::GeneralizedTotaliser::Factory(),
////        "Dynamic", true, false, start_penalty, 1);
//
////    test_setting(dir + file, file, log_dir,
////    (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory *) new
////    ::Pumpkin::GeneralizedTotliserSumRoot::Factory(), "Dynamic root", true,
////    true, start_penalty,   0.01);
//    //test_setting(dir + file, file, log_dir,
//  //  (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory *) new
//  //  ::Pumpkin::GeneralizedTotaliser::Factory(), "Dynamic", true, false,
//  //  start_penalty, 1);
////      test_setting(dir + file, file, log_dir,
////    (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory *) new
////    ::Pumpkin::GeneralizedTotaliser::Factory(), "Incremental", true, true,
////    start_penalty, 1);
//      //test_setting(dir + file, file, log_dir,
//  //  (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory *) new
//  //  ::Pumpkin::StaticGeneralizedTotaliser::Factory(), "Static order
//  //  Incremental", true, true, start_penalty, 0);
//      test_setting(dir + file,
//    file, log_dir,
//    (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory *) new
//    ::Pumpkin::PropagatorEncoder<Pumpkin::PseudoBooleanConstraint>::Factory(),
//    "Propagator", false, false, start_penalty, 1);
////  test_setting(
////      dir + file, file, log_dir,
////      (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory
////           *)new ::Pumpkin::GeneralizedTotaliser::Factory(),
////      "Encoder", false, false, start_penalty, 0);
//}
//
//enum solver_type {
//  encoder,
//  dynamic,
//  incremental,
//  propagator,
//  static_incremental,
//  static_dynamic,
//  top_down,
//};

using namespace simple_sat_solver::ctt;
using namespace simple_sat_solver::benchmark;
int main(int argc, char *argv[]) {

  CttBenchmark benchmark;
  if (argc >= 4) {
    benchmark.Main(argc, argv);
  } else {
    std::string test_file = "/home/jens/CLionProjects/SimpleSatSolver/data/ctt/comp07.ctt";
    std::string log_dir="/home/jens/CLionProjects/SimpleSatSolver/data/ctt/logs_a4";
    std::string test_dir = "/home/jens/CLionProjects/SimpleSatSolver/data/ctt/";

    benchmark.solver_type_ = simple_sat_solver::benchmark::SolverType::ENCODER;
    benchmark.problem_file_full_path_ =test_file;
    benchmark.log_dir_ = log_dir;
    benchmark.delay_factor_ = 0.0;
    benchmark.start_penalty_ = 100000;

    benchmark.Main();
  }
  return 0;
//  if (argc >= 4) {
//    solver_type s = (solver_type)atoi(argv[1]);
//    std::string test_file = argv[2];
//    std::string log_dir = argv[3];
//    int add_delay_i = 10;
//    if (argc >= 5) {
//      add_delay_i = atoi(argv[4]);
//    }
//    double add_delay = ((double)add_delay_i) / 10.0;
//    int start_penalty = 100000;
//    ::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory *factory;
//    switch (s) {
//    case encoder:
//      test_setting(
//          test_file, "test", log_dir,
//          (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory
//               *)new ::Pumpkin::GeneralizedTotaliser::Factory(),
//          "Encoder", false, false, start_penalty, add_delay);
//      break;
//    case dynamic:
//      test_setting(
//          test_file, "test", log_dir,
//          (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory
//               *)new ::Pumpkin::GeneralizedTotaliser::Factory(),
//          "Dynamic", true, false, start_penalty, add_delay);
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
//               *)new ::Pumpkin::StaticGeneralizedTotaliser::Factory(),
//          "StaticIncremental", true, true, start_penalty, add_delay);
//      break;
//    case static_dynamic:
//      test_setting(
//          test_file, "test", log_dir,
//          (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory
//               *)new ::Pumpkin::StaticGeneralizedTotaliser::Factory(),
//          "StaticDynamic", true, false, start_penalty, add_delay);
//      break;
//    case top_down:
//      test_setting(test_file, "test", log_dir,
//                   (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory *) new
//                       ::Pumpkin::GeneralizedTotliserSumRoot::Factory(), "Top Down", true,
//                   true, start_penalty,   add_delay / 10.0);
//    default:
//      return 1;
//    }
//
//  } else {
//
//    //  simple_sat_solver::ctt::Test("../../../data/ctt/toyexample.ctt");
////     test_file("toyexample.ctt");
//        for (int i = 2; i <= 21; ++i) {
////    {
////      int i = 11;
//      char str[2];
//      std::sprintf(str, "%02d", i);
//      std::string s;
//      for (int k = 0; k < 2; ++k)
//        s += str[k];
//      std::cout << str << std::endl;
//      std::cout << s << std::endl;
//      test_file("comp" + s + ".ctt", 100000);
//    }
//    //      test_file("comp01.ctt", 10090);
//    //      test_file("comp02.ctt", 19000);
//    //      test_file("comp03.ctt", 10900);
//    //      test_file("comp04.ctt", 10900);
//    //    test_file("comp01.ctt", 10090);
//    //    test_file("comp06.ctt", 10090);
//    //    test_file("comp16.ctt", 1009000);
//    //      test_file("comp12.ctt", 10090);
//
//    //  ::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory *
//    //  encoder_factory = new ::Pumpkin::GeneralizedTotaliser::Factory();
//    ////    ::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory *
//    /// encoder_factory = new ::Pumpkin::PropagatorEncoder::Factory();
//    //  encoder_factory->add_dynamic_ = false;
//    //  encoder_factory->add_incremetal_ = false;
//    //  simple_sat_solver::ctt::Test("../../../data/ctt",file , encoder_factory,
//    //  "Incremental");
//  }
}
