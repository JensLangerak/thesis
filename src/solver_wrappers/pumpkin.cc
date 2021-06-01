//
// Created by jens on 15-10-20.
//

#include "pumpkin.h"

#include "../pumpkin/Utilities//boolean_literal.h"
#include "../pumpkin/Utilities//problem_specification.h"
#include "../pumpkin/Utilities//solver_parameters.h"
#include "../pumpkin/Engine/constraint_optimisation_solver.h"

#include "../logger/logger.h"
#include "../pseudo_boolean/opb_parser.h"
#include "../pumpkin/Propagators/Dynamic/Encoders/propagator_encoder.h"
#include "../sat/constraints/cardinality_constraint.h"
#include "../sat/constraints/sum_constraint.h"
#include "../sat/encoders/totaliser_encoder.h"
namespace simple_sat_solver::solver_wrappers {
using namespace Pumpkin;

bool Pumpkin::check_sol = false;
std::vector<bool> Pumpkin::check_solution_;

bool Pumpkin::Solve(const sat::SatProblem &p2) {
//  solved_ = false;
//  sat::SatProblem p = p2;
//
//  ProblemSpecification problem = ConvertProblem(p);
//  SolverParameters params;
//  params.bump_decision_variables = true;
//  ConstraintSatisfactionSolver solver(problem, params);
//  SolverOutput solver_output = solver.Solve(600);
//  std::cout << "Sol found: " << (solver_output.timeout_happened ? "F" : "T") <<std::endl;
//  solved_ = solver_output.HasSolution();
//  if (solved_) {
//    simple_sat_solver::logger::Logger::Log2("SAT");
//  } else if(!solver_output.timeout_happened) {
//    simple_sat_solver::logger::Logger::Log2("UNSAT");
//  } else {
//    simple_sat_solver::logger::Logger::Log2("TIMEOUT");
//  }
//  assert(solver_output.solution.size() - 1 >= p.GetNrVars());
//  if (solved_) {
//    solution_ = std::vector<bool>();
//    for (int i = 1; i < solver_output.solution.size(); ++i)
//      solution_.push_back(solver_output.solution[i]);
//  }
//
////  if (solver.state_.propagator_cardinality_.trigger_count_ != 0)
//    std::cout << "count: "
//              << solver.state_.propagator_cardinality_.trigger_count_
//              << std::endl;
////    solver.state_.propagator_cardinality_.cardinality_database_.permanent_constraints_[0]->encoder_->PrintInfo();
//
//  return solved_;
    return true;
}
std::vector<bool> Pumpkin::GetSolution() const { return solution_; }

bool Pumpkin::Optimize(const sat::SatProblem &p2) {

  solved_ = false;
//  sat::SatProblem p = p2;
//
//  ProblemSpecification problem = ConvertProblem(p);
//  SolverParameters params;
//  params.bump_decision_variables = true;
//  ConstraintOptimisationSolver solver(problem, params);
//  solver.optimisation_encoding_factory = optimisation_encoder_factory_; // TODO
////  SolverOutput solver_output = solver.Solve(std::numeric_limits<double>::max());
//  solver.start_upper_bound_ = start_uppberboud_;
////  if (check_sol) {
////    for (int i = 0; i < problem.num_Boolean_variables_; ++i) {
////
////      ::Pumpkin::BooleanLiteral l =
////          ::Pumpkin::BooleanLiteral(BooleanVariable(i + 1), check_solution_[i]);
////      solver.constrained_satisfaction_solver_.state_.AddUnitClause(l);
////    }
////
//////    SolverOutput solver_output =
//////        solver.constrained_satisfaction_solver_.Solve(190);
//////    assert(solver_output.HasSolution());
////  }
//    SolverOutput solver_output = solver.Solve(600);
//  solved_ = solver_output.HasSolution();
//
//  assert(solver_output.solution.size() - 1 >= p.GetNrVars());
//  if (solved_) {
//    solution_ = std::vector<bool>();
//    for (int i = 1; i < solver_output.solution.size(); ++i)
//      solution_.push_back(solver_output.solution[i]);
//  }
//
////  if (solver.constrained_satisfaction_solver_.state_.propagator_sum_
////      .trigger_count_ != 0)
////    std::cout << "countsum: "
////              << solver.constrained_satisfaction_solver_.state_
////                  .propagator_sum_.trigger_count_
////              << std::endl;
//  if (solver.constrained_satisfaction_solver_.state_.propagator_cardinality_
//          .trigger_count_ != 0)
//    std::cout << "count: "
//              << solver.constrained_satisfaction_solver_.state_
//                     .propagator_cardinality_.trigger_count_
//              << std::endl;
//  if (check_sol && false) {
//    solver.constrained_satisfaction_solver_.state_.Reset();
//    for (int i = 0; i < problem.num_Boolean_variables_; ++i) {
//
//      ::Pumpkin::BooleanLiteral l =
//          ::Pumpkin::BooleanLiteral(BooleanVariable(i+1), check_solution_[i]);
//      solver.constrained_satisfaction_solver_.state_.AddUnitClause(l);
//    }
//
//      SolverOutput solver_output = solver.constrained_satisfaction_solver_.Solve(30);
//      assert(solver_output.HasSolution());
//
//  }else {
//    check_solution_ = solution_;
//    check_sol = true;
//  }


  return solved_;
}
ProblemSpecification Pumpkin::ConvertProblem(sat::SatProblem &p) {
  ProblemSpecification problem;
//  problem.num_Boolean_variables_ = p.GetNrVars();
//  for (const auto &c : p.GetClauses()) {
//    std::vector<::Pumpkin::BooleanLiteral> clause;
//    for (sat::Lit l : c) {
//      ::Pumpkin::BooleanLiteral lit =
//          ::Pumpkin::BooleanLiteral(BooleanVariable(l.x + 1), !l.complement);
//      clause.push_back(lit);
//    }
//    problem.AddClause(clause);
//  }
//
//  for (sat::IConstraint * c : p.GetConstraints()) {
//    if (sat::CardinalityConstraint *car =
//            dynamic_cast<sat::CardinalityConstraint *>(c)) {
//      std::vector<::Pumpkin::BooleanLiteral> lits;
//      std::vector<uint32_t> weights;
//      for (sat::Lit l : car->lits) {
//        ::Pumpkin::BooleanLiteral lit =
//            ::Pumpkin::BooleanLiteral(BooleanVariable(l.x + 1), !l.complement);
//        lits.push_back(lit);
//        weights.push_back(1);
//      }
//      assert(car->min == 0);
//      problem.pseudo_boolean_constraints_.push_back(::Pumpkin::PseudoBooleanConstraint(lits,weights,car->max, constraint_encoder_factory_));
//      //    }
//    } else if (sat::SumConstraint *car =
//                   dynamic_cast<sat::SumConstraint *>(c)) {
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
//
//      assert(false);
//      // TODO
//      //      problem.sum_constraints_.push_back(
//      //          ::Pumpkin::SumConstraint(inputs, outputs, encoder_factory_));
//    } else if (sat::PseudoBooleanConstraint *car =
//          dynamic_cast<sat::PseudoBooleanConstraint *>(c)) {
//      std::vector<::Pumpkin::BooleanLiteral> lits;
//      std::vector<uint32_t> weights;
//      for (sat::WeightedLit l : car->weighted_lits_) {
//        ::Pumpkin::BooleanLiteral lit =
//            ::Pumpkin::BooleanLiteral(BooleanVariable(l.l.x + 1), !l.l.complement);
//        lits.push_back(lit);
//        weights.push_back(l.w);
//      }
//
//      problem.pseudo_boolean_constraints_.push_back(::Pumpkin::PseudoBooleanConstraint(lits,weights,car->max_, constraint_encoder_factory_));
//
//    } else {
//      assert(false);
//    }
//  }
//    //TODO
//    auto t = p.GetMinimizeLit();
//    for (auto l : p.GetMinimizeLit()) {
//      problem.objective_literals_.push_back(WeightedLiteral(
//          ::Pumpkin::BooleanLiteral(BooleanVariable(l.l.x + 1), true), l.w));
//  }
  return problem;
}
} // namespace simple_sat_solver::solver_wrappers
