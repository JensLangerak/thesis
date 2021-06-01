//
// Created by jens on 31-05-21.
//

#include "benchmark.h"
#include "../logger/logger.h"
#include "../pumpkin/Utilities/solver_parameters.h"
#include "../pumpkin/Engine/constraint_optimisation_solver.h"
#include "../pumpkin/Engine/constraint_satisfaction_solver.h"
#include "../pumpkin/Propagators/Dynamic/PseudoBoolean/pseudo_boolean_adder.h"
#include "../pumpkin/Propagators/Dynamic/Encoders/generalized_totaliser.h"
#include <cassert>
namespace simple_sat_solver::benchmark {
using namespace Pumpkin;
void Benchmark::BenchmarkMain(int argc, char **argv, Benchmark *benchmark) {
  assert(argc >= 4);
  benchmark->Init(argc, argv);
  BenchmarkMain(benchmark);
}
void Benchmark::BenchmarkMain( Benchmark *benchmark) {
  benchmark->WriteHeader();
  ProblemSpecification * problem = benchmark->GetProblem();

  SolverParameters parameters;
  ParameterHandler parameter_handler = ConstraintOptimisationSolver::CreateParameterHandler();
  parameter_handler.optimisation_constraint_wrapper_ = benchmark->CreatePbConstraintWrapper();
  parameter_handler.SetIntegerParameter("seed", 0);


  SolverOutput solver_output;
  if (problem->weighted_literals_.empty()) {
    ConstraintSatisfactionSolver solver(problem, parameter_handler);
    solver_output = solver.Solve(benchmark->solve_time_);
  } else {
    parameter_handler.SetStringParameter("file", benchmark->problem_file_full_path_);
    ConstraintOptimisationSolver solver(problem, parameter_handler);
//    ConstraintOptimisationSolver solver(parameter_handler);
    solver_output = solver.Solve(benchmark->solve_time_, 0);
    std::cout << "Penalty " << std::to_string(solver_output.cost) << std::endl;
  }

  benchmark->CheckSolutionCorrectness(problem, solver_output);

  bool solved = solver_output.HasSolution();

  if (solved)
    std::cout << "Solution found" << std::endl;

  simple_sat_solver::logger::Logger::End();

  delete parameter_handler.optimisation_constraint_wrapper_;
  parameter_handler.optimisation_constraint_wrapper_ = nullptr;
  delete problem;


}
void Benchmark::Init(int argc, char **argv) {
  solver_type_ = (SolverType)atoi(argv[1]);
  problem_file_full_path_ = argv[2];
  log_dir_ = argv[3];
  int add_delay_i = 10;
  if (argc >=5)
    add_delay_i = atoi(argv[4]);
  delay_factor_ = ((double) add_delay_i) / 10.0;
  start_penalty_ = 100000;

}
void Benchmark::WriteHeader() {
  simple_sat_solver::logger::Logger::StartNewLog(log_dir_,"test");
  simple_sat_solver::logger::Logger::Log2("File: " + problem_file_full_path_);
  simple_sat_solver::logger::Logger::Log2("Encoder: " + GetEncoderName());
  simple_sat_solver::logger::Logger::Log2("Delay factor: " + std::to_string(delay_factor_));
}
std::string Benchmark::GetEncoderName() {
  switch (solver_type_) {
  case SolverType::ENCODER:
    return "encoder";
  case SolverType::DYNAMIC:
    return "dynamic";
  case SolverType::INCREMENTAL:
    return "incremental";
  case SolverType::PROPAGATOR:
    return "propagator";
  }
}
Pumpkin::IConstraintAdder<Pumpkin::PseudoBooleanConstraint> *
Benchmark::CreatePbConstraintWrapper() {
  switch (solver_type_) {

  case SolverType::ENCODER:
  case SolverType::DYNAMIC:
  case SolverType::INCREMENTAL:
  case SolverType::PROPAGATOR: {
    PropagatorPseudoBoolean2 * propagator = new PropagatorPseudoBoolean2(0);
    auto pb_adder = new PseudoBooleanAdder(propagator);
    switch (solver_type_) {

    case SolverType::ENCODER:
      pb_adder->encoder_factory = new GeneralizedTotaliser::Factory(Pumpkin::IEncoder<PseudoBooleanConstraint>::START, delay_factor_);
      break;
    case SolverType::DYNAMIC:
      pb_adder->encoder_factory = new GeneralizedTotaliser::Factory(Pumpkin::IEncoder<PseudoBooleanConstraint>::DYNAMIC, delay_factor_);
      break;
    case SolverType::INCREMENTAL:
      pb_adder->encoder_factory = new GeneralizedTotaliser::Factory(Pumpkin::IEncoder<PseudoBooleanConstraint>::INCREMENTAL, delay_factor_);
      break;
    case SolverType::PROPAGATOR:
      pb_adder->encoder_factory = new GeneralizedTotaliser::Factory(Pumpkin::IEncoder<PseudoBooleanConstraint>::NEVER, delay_factor_);
      break;
    }
    return pb_adder;
  }
  }
}
}
