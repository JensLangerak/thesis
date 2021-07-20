//
// Created by jens on 31-05-21.
//

#include "benchmark.h"
#include "../logger/logger.h"
#include "../pseudo_boolean/opb_parser.h"
#include "../pumpkin/Engine/constraint_optimisation_solver.h"
#include "../pumpkin/Engine/constraint_satisfaction_solver.h"
#include "../pumpkin/Propagators/Dynamic/PseudoBoolean/propagator_pseudo_boolean_2.h"
#include "../pumpkin/Propagators/Dynamic/Encoders/generalized_totliser_sum_root.h"
#include "../pumpkin/Propagators/Dynamic/Encoders/generalized_totaliser.h"
#include "../pumpkin/Propagators/Dynamic/PseudoBoolean/pseudo_boolean_adder.h"
#include "../pumpkin/Propagators/Dynamic/Pairs/pb_pairs_adder.h"
#include "../pumpkin/Propagators/Dynamic/BottomLayers//bottom_layers_adder.h"
#include "../pumpkin/Utilities/solver_parameters.h"
#include "../sat/constraints/cardinality_constraint.h"
#include "../sat/constraints/sum_constraint.h"
#include <cassert>
#include <chrono>
#include <thread>

namespace simple_sat_solver::benchmark {
using namespace Pumpkin;
void Benchmark::Main(int argc, char **argv) {
  assert(argc >= 4);
  Init(argc, argv);
  Main();
}


void Benchmark::MainOrder() {
  WriteHeader();
  simple_sat_solver::logger::Logger::Log2("RunTime: " + std::to_string(probe_time_));

  optimisation_adder_ = CreatePbConstraintWrapper(solver_type_);
  pb_adder_ = CreatePbConstraintWrapper(SolverType::ENCODER);
//  pb_adder_ = CreatePbConstraintWrapper(solver_type_);
  ProblemSpecification problem = GetProblem();


  ParameterHandler parameter_handler = ConstraintOptimisationSolver::CreateParameterHandler();
  parameter_handler.optimisation_constraint_wrapper_ = optimisation_adder_;
  parameter_handler.SetIntegerParameter("seed", 0);


  SolverOutput solver_output;
  ProblemSpecification problem_test;
  if (problem.weighted_literals_.empty()) {
    ConstraintSatisfactionSolver solver(&problem, parameter_handler);
    solver_output = solver.Solve(probe_time_);

    problem_test = CreateProblemOrderLiterals(problem, solver.state_);
  } else {
    parameter_handler.SetStringParameter("file", problem_file_full_path_);
    ConstraintOptimisationSolver solver(&problem, parameter_handler);
//    ConstraintOptimisationSolver solver(parameter_handler);
    solver_output = solver.Solve(probe_time_, 0);
    std::cout << "Penalty " << std::to_string(solver_output.cost) << std::endl;
    problem_test = CreateProblemOrderLiterals(problem, solver.constrained_satisfaction_solver_.state_);
  }

  CheckSolutionCorrectness(&problem, solver_output);

  bool solved = solver_output.HasSolution();

  if (solved)
    std::cout << "Solution found" << std::endl;
  std::cout << "Probing done" << std::endl;

  simple_sat_solver::logger::Logger::End();

  delete optimisation_adder_;
  parameter_handler.optimisation_constraint_wrapper_ = nullptr;
  std::this_thread::sleep_for(std::chrono::seconds (1));

  WriteHeader();
  simple_sat_solver::logger::Logger::Log2("ProbeTime: " + std::to_string(probe_time_));

  optimisation_adder_ = CreatePbConstraintWrapper(solver_type_);

  parameter_handler = ConstraintOptimisationSolver::CreateParameterHandler();
  parameter_handler.optimisation_constraint_wrapper_ = optimisation_adder_;
  parameter_handler.SetIntegerParameter("seed", 0);


  if (problem.weighted_literals_.empty()) {
    ConstraintSatisfactionSolver solver(&problem_test, parameter_handler);
    solver_output = solver.Solve(solve_time_);
  } else {
    parameter_handler.SetStringParameter("file", problem_file_full_path_);
    ConstraintOptimisationSolver solver(&problem_test, parameter_handler);
    solver_output = solver.Solve(solve_time_, 0);
    simple_sat_solver::logger::Logger::Log2("Penalty: " + std::to_string(solver_output.cost));

    std::cout << "Penalty " << std::to_string(solver_output.cost) << std::endl;
  }

  CheckSolutionCorrectness(&problem, solver_output);

  solved = solver_output.HasSolution();

  if (solved)
    std::cout << "Solution found" << std::endl;

  if (solver_output.ProvenInfeasible())
    simple_sat_solver::logger::Logger::Log2("Done: UNSAT");
  if (solver_output.HasSolution())
    simple_sat_solver::logger::Logger::Log2("Done: SAT");

  simple_sat_solver::logger::Logger::End();

  delete optimisation_adder_;
  delete pb_adder_;
  parameter_handler.optimisation_constraint_wrapper_ = nullptr;

}


void Benchmark::Main() {
//  MainOrder();
//  return;
  WriteHeader();
  optimisation_adder_ = CreatePbConstraintWrapper(solver_type_);
//  pb_adder_ = CreatePbConstraintWrapper(SolverType::PROPAGATOR);
  pb_adder_ = CreatePbConstraintWrapper(solver_type_);
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
    simple_sat_solver::logger::Logger::Log2("Penalty: " + std::to_string(solver_output.cost));

    std::cout << "Penalty " << std::to_string(solver_output.cost) << std::endl;
  }

  CheckSolutionCorrectness(&problem, solver_output);

  bool solved = solver_output.HasSolution();

  if (solved)
    std::cout << "Solution found" << std::endl;

  if (solver_output.ProvenInfeasible())
    simple_sat_solver::logger::Logger::Log2("Done: UNSAT");
  if (solver_output.HasSolution())
    simple_sat_solver::logger::Logger::Log2("Done: SAT");

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
  probe_time_ = add_delay_i;
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
  case SolverType::BOTTOMLAYERS:
    return "bottomlayers";
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
  case SolverType::BOTTOMLAYERS: {
    auto prop = new PropagatorBottomLayers(0);
    auto adder = new BottomLayersAdder(prop);
    return adder;
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
struct WeightedActiveLiteral {
  Pumpkin::BooleanLiteral lit;
  double activity;
  uint32_t weight;
  WeightedActiveLiteral(Pumpkin::BooleanLiteral lit, uint32_t weight, double activiy) : lit(lit), weight(weight), activity(activiy) {}
  bool operator< (const WeightedActiveLiteral rhs) {
    if (weight != rhs.weight)
      return weight > rhs.weight;
    return activity > rhs.activity;
  }
};
struct WeightedCountLiteral {
  Pumpkin::BooleanLiteral lit;
  int count;
  uint32_t weight;
  WeightedCountLiteral(Pumpkin::BooleanLiteral lit, uint32_t weight, int count) : lit(lit), weight(weight), count(count) {}
  bool operator< (const WeightedCountLiteral rhs) {
    if (weight != rhs.weight)
      return weight > rhs.weight;
    return count > rhs.count;
  }
};
Pumpkin::ProblemSpecification Benchmark::CreateProblemOrderLiterals(
    Pumpkin::ProblemSpecification specification, Pumpkin::SolverState &state) {
  bool use_count = false;
  auto old_pb_adder = pb_adder_;
  Pumpkin::PropagatorPseudoBoolean2* propagator_adder =
      dynamic_cast<PropagatorPseudoBoolean2 *>(old_pb_adder->GetPropagator());
  pb_adder_ = CreatePbConstraintWrapper(SolverType::ENCODER);
  ProblemSpecification problem2 = ProblemSpecification();


  problem2.num_Boolean_variables_ = specification.num_Boolean_variables_;
  problem2.num_binary_clauses_ = specification.num_binary_clauses_;
  problem2.num_literals_in_other_clauses_ = specification.num_literals_in_other_clauses_;
  problem2.num_other_clauses_ = specification.num_other_clauses_;
  problem2.num_ternary_clauses_ = specification.num_ternary_clauses_;

  for (auto b : specification.unit_clauses_)
    problem2.unit_clauses_.push_back(b);

  for (auto c : specification.clauses_) {
    std::vector<Pumpkin::BooleanLiteral> clause;
    for (auto b : c)
      clause.push_back(b);
    problem2.clauses_.push_back(c);
  }

    for (int i = 0; i < specification.pseudo_boolean_constraints_.size(); ++i) {
      auto c= specification.pseudo_boolean_constraints_[i];
    std::vector<WeightedActiveLiteral> wlits;
    std::vector<WeightedCountLiteral> clitsl;
    for (int i=0; i < c.literals.size(); ++i ) {
      wlits.push_back(WeightedActiveLiteral(c.literals[i],c.coefficients[i], state.variable_selector_.GetActiviy(c.literals[i].Variable())));
      int count = propagator_adder->pseudo_boolean_database_.permanent_constraints_[i]->GetLitCount(c.literals[i]);
      clitsl.push_back(WeightedCountLiteral(c.literals[i],c.coefficients[i], count));
    }
    std::sort(wlits.begin(), wlits.end());
  std::sort(clitsl.begin(), clitsl.end());

  std::vector<Pumpkin::BooleanLiteral> lits;
  std::vector<uint32_t > weighs;
  if (use_count) {
  for (auto wl : clitsl) {
    lits.push_back(wl.lit);
    weighs.push_back(wl.weight);
  } } else {
    for (auto wl : wlits) {
      lits.push_back(wl.lit);
      weighs.push_back(wl.weight);
    }
  }
  problem2.pseudo_boolean_constraints_.push_back(PseudoBooleanConstraint(lits, weighs, c.right_hand_side, pb_adder_));

  }


  auto old_opt_pb_adder = optimisation_adder_;
  Pumpkin::PropagatorPseudoBoolean2* propagator_opb =
      dynamic_cast<PropagatorPseudoBoolean2 *>(old_opt_pb_adder->GetPropagator());

  std::vector<WeightedActiveLiteral> wlits;
  std::vector<WeightedCountLiteral> clitsl;
  for (auto pw : specification.weighted_literals_) {
      wlits.push_back(WeightedActiveLiteral(pw.literal, pw.weight, state.variable_selector_.GetActiviy(pw.literal.Variable())));
      int count = propagator_opb->pseudo_boolean_database_.permanent_constraints_.back()->GetLitCount(pw.literal);
    clitsl.push_back(WeightedCountLiteral(pw.literal, pw.weight, count));

  }

  std::sort(wlits.begin(), wlits.end());
  std::sort(clitsl.begin(), clitsl.end());
  if (use_count) {
    for (auto wl : clitsl)
      problem2.weighted_literals_.push_back(
          PairWeightLiteral(wl.lit, wl.weight));
  } else {
    for (auto wl : wlits)
      problem2.weighted_literals_.push_back(
          PairWeightLiteral(wl.lit, wl.weight));
  }
  delete old_pb_adder;
  return problem2;
}
}
