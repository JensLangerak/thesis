//
// Created by jens on 31-05-21.
//

#include "benchmark.h"
#include "../logger/logger.h"
#include "../pseudo_boolean/opb_parser.h"
#include "../pumpkin/Engine/constraint_optimisation_solver.h"
#include "../pumpkin/Engine/constraint_satisfaction_solver.h"
#include "../pumpkin/Propagators/Dynamic/Encoders/generalized_totliser_sum_root.h"
#include "../pumpkin/Propagators/Dynamic/Encoders/generalized_totaliser.h"
#include "../pumpkin/Propagators/Dynamic/PseudoBoolean/pseudo_boolean_adder.h"
#include "../pumpkin/Propagators/Dynamic/Pairs/pb_pairs_adder.h"
#include "../pumpkin/Utilities/solver_parameters.h"
#include "../sat/constraints/cardinality_constraint.h"
#include "../sat/constraints/sum_constraint.h"
#include <cassert>
namespace simple_sat_solver::benchmark {
using namespace Pumpkin;
void Benchmark::Main(int argc, char **argv) {
  assert(argc >= 4);
  Init(argc, argv);
  Main();
}
void Benchmark::Main() {
  WriteHeader();
  optimisation_adder_ = CreatePbConstraintWrapper(solver_type_);
  pb_adder_ = CreatePbConstraintWrapper(SolverType::PROPAGATOR);
  ProblemSpecification problem = GetProblem();


  ParameterHandler parameter_handler = ConstraintOptimisationSolver::CreateParameterHandler();
  parameter_handler.optimisation_constraint_wrapper_ = optimisation_adder_;
  parameter_handler.SetIntegerParameter("seed", 0);


  SolverOutput solver_output;
  if (problem.weighted_literals_.empty()) {
    ConstraintSatisfactionSolver solver(&problem, parameter_handler);
    solver_output = solver.Solve(solve_time_);
  } else {
    parameter_handler.SetStringParameter("file", problem_file_full_path_);
    ConstraintOptimisationSolver solver(&problem, parameter_handler);
//    ConstraintOptimisationSolver solver(parameter_handler);
    solver_output = solver.Solve(solve_time_, 0);
    std::cout << "Penalty " << std::to_string(solver_output.cost) << std::endl;
  }

  CheckSolutionCorrectness(&problem, solver_output);

  bool solved = solver_output.HasSolution();

  if (solved)
    std::cout << "Solution found" << std::endl;

  simple_sat_solver::logger::Logger::End();

  delete optimisation_adder_;
  delete pb_adder_;
  parameter_handler.optimisation_constraint_wrapper_ = nullptr;


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

  auto timenow =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::string time_message =  std::ctime(&timenow);
  simple_sat_solver::logger::Logger::Log2("Starttime:" + time_message);

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
  case SolverType::PAIRS:
    return "pairs";
  case SolverType::TOPDOWN:
    return "topdown";
  }
}
Pumpkin::IConstraintAdder<Pumpkin::PseudoBooleanConstraint> *
Benchmark::CreatePbConstraintWrapper(SolverType solver_type) {
  switch (solver_type) {

  case SolverType::ENCODER:
  case SolverType::DYNAMIC:
  case SolverType::INCREMENTAL:
  case SolverType::PROPAGATOR:
  case SolverType::TOPDOWN:
  {
    PropagatorPseudoBoolean2 * propagator = new PropagatorPseudoBoolean2(0);
    auto pb_adder = new PseudoBooleanAdder(propagator);
    switch (solver_type) {

    case SolverType::ENCODER:
      pb_adder->encoder_factory = new GeneralizedTotaliser::Factory(START, delay_factor_);
      break;
    case SolverType::DYNAMIC:
      pb_adder->encoder_factory = new GeneralizedTotaliser::Factory(DYNAMIC, delay_factor_);
      break;
    case SolverType::INCREMENTAL:
      pb_adder->encoder_factory = new GeneralizedTotaliser::Factory(INCREMENTAL, delay_factor_);
      break;
    case SolverType::PROPAGATOR:
      pb_adder->encoder_factory = new GeneralizedTotaliser::Factory(NEVER, delay_factor_);
      break;
    case SolverType::TOPDOWN:
      pb_adder->encoder_factory = new GeneralizedTotliserSumRoot::Factory(DYNAMIC, delay_factor_); //TODO not sure what the best way is to handle this method
      break;
    default:
      assert(false);
    }
    return pb_adder;
  }
  case SolverType::PAIRS: {
    auto prop = new PropagatorPbPairs(0);
    auto pb_adder = new PbPairsAdder(prop);
    return pb_adder;
  }
  }
}
Pumpkin::ProblemSpecification
Benchmark::ConvertSatToPumpkin(sat::SatProblem *p) {
  ProblemSpecification problem;
  problem.num_Boolean_variables_ = p->GetNrVars();
  for (const auto &c : p->GetClauses()) {
    std::vector<::Pumpkin::BooleanLiteral> clause;
    for (sat::Lit l : c) {
      ::Pumpkin::BooleanLiteral lit =
          ::Pumpkin::BooleanLiteral(BooleanVariable(l.x + 1), !l.complement);
      clause.push_back(lit);
    }
    problem.AddClause(clause);
  }

  for (sat::IConstraint * c : p->GetConstraints()) {
    if (sat::CardinalityConstraint *car =
            dynamic_cast<sat::CardinalityConstraint *>(c)) {
      std::vector<::Pumpkin::BooleanLiteral> lits;
      std::vector<uint32_t> weights;
      for (sat::Lit l : car->lits) {
        ::Pumpkin::BooleanLiteral lit =
            ::Pumpkin::BooleanLiteral(BooleanVariable(l.x + 1), !l.complement);
        lits.push_back(lit);
        weights.push_back(1);
      }
      assert(car->min == 0);
      problem.pseudo_boolean_constraints_.push_back(::Pumpkin::PseudoBooleanConstraint(lits,weights,car->max, pb_adder_));
      //    }
    } else if (sat::SumConstraint *car =
                   dynamic_cast<sat::SumConstraint *>(c)) {
      assert(false);
//      std::vector<::Pumpkin::BooleanLiteral> inputs;
//      for (sat::Lit l : car->input_lits_) {
//        ::Pumpkin::BooleanLiteral lit =
//            ::Pumpkin::BooleanLiteral(BooleanVariable(l.x + 1), !l.complement);
//        inputs.push_back(lit);
//      }
//      std::vector<::Pumpkin::BooleanLiteral> outputs;
//      for (sat::Lit l : car->output_lits_) {
//        ::Pumpkin::BooleanLiteral lit =
//            ::Pumpkin::BooleanLiteral(BooleanVariable(l.x + 1), !l.complement);
//        outputs.push_back(lit);
//      }

      assert(false);
      // TODO
      //      problem.sum_constraints_.push_back(
      //          ::Pumpkin::SumConstraint(inputs, outputs, encoder_factory_));
    } else if (sat::PseudoBooleanConstraint *car =
          dynamic_cast<sat::PseudoBooleanConstraint *>(c)) {
      std::vector<::Pumpkin::BooleanLiteral> lits;
      std::vector<uint32_t> weights;
      for (sat::WeightedLit l : car->weighted_lits_) {
        ::Pumpkin::BooleanLiteral lit =
            ::Pumpkin::BooleanLiteral(BooleanVariable(l.l.x + 1), !l.l.complement);
        lits.push_back(lit);
        weights.push_back(l.w);
      }

      problem.pseudo_boolean_constraints_.push_back(::Pumpkin::PseudoBooleanConstraint(lits,weights,car->max_, pb_adder_));

    } else {
      assert(false);
    }
  }
    auto t = p->GetMinimizeLit();
    for (auto l : p->GetMinimizeLit()) {
      problem.weighted_literals_.push_back(PairWeightLiteral(
          ::Pumpkin::BooleanLiteral(BooleanVariable(l.l.x + 1), true), l.w));
  }
  return problem;
}
}
