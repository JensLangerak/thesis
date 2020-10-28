//
// Created by jens on 15-10-20.
//

#include "pumpkin.h"

#include "../pumpkin/Basic Data Structures/boolean_literal.h"
#include "../pumpkin/Basic Data Structures/problem_specification.h"
#include "../pumpkin/Basic Data Structures/solver_parameters.h"
#include "../pumpkin/Engine/constraint_optimisation_solver.h"

#include "../pumpkin/Propagators/Cardinality/Encoders/incremental_sequential_encoder.h"
#include "../pumpkin/Propagators/Cardinality/Encoders/sequential_encoder.h"
#include "../pumpkin/Propagators/Cardinality/Encoders/totaliser_encoder.h"
#include "../sat/encoders/totaliser_encoder.h"
namespace simple_sat_solver::solver_wrappers {
using namespace Pumpkin;

bool Pumpkin::Solve(const sat::SatProblem &p2) {
  solved_ = false;
  sat::SatProblem p = p2;

  ProblemSpecification problem = ConvertProblem(p);
  SolverParameters params;
  params.bump_decision_variables = true;
  ConstraintSatisfactionSolver solver(problem, params);
  SolverOutput solver_output = solver.Solve(std::numeric_limits<double>::max());
  solved_ = solver_output.HasSolution();

  assert(solver_output.solution.size() - 1 >= p.GetNrVars());
  if (solved_) {
    solution_ = std::vector<bool>();
    for (int i = 1; i < solver_output.solution.size(); ++i)
      solution_.push_back(solver_output.solution[i]);
  }

  if (solver.state_.propagator_cardinality_.trigger_count_ != 0)
    std::cout << "count: "
              << solver.state_.propagator_cardinality_.trigger_count_
              << std::endl;
  if (solver.state_.propagator_cardinality2_.trigger_count_ != 0) {
    int max_trigger = 0;
    for (auto c : solver.state_.propagator_cardinality2_.cardinality_database_
                      .permanent_constraints_)
      max_trigger = std::max(max_trigger, c->trigger_count_);
    std::cout << "count2: "
              << solver.state_.propagator_cardinality2_.trigger_count_
              << "  -  " << max_trigger << std::endl;
  }
  return solved_;
}
std::vector<bool> Pumpkin::GetSolution() const { return solution_; }

bool Pumpkin::Optimize(const sat::SatProblem &p2) {

  solved_ = false;
  sat::SatProblem p = p2;

  ProblemSpecification problem = ConvertProblem(p);
  SolverParameters params;
  params.bump_decision_variables = true;
  ConstraintOptimisationSolver solver(problem, params);
  SolverOutput solver_output = solver.Solve(std::numeric_limits<double>::max());
  solved_ = solver_output.HasSolution();

  assert(solver_output.solution.size() - 1 >= p.GetNrVars());
  if (solved_) {
    solution_ = std::vector<bool>();
    for (int i = 1; i < solver_output.solution.size(); ++i)
      solution_.push_back(solver_output.solution[i]);
  }

  if (solver.constrained_satisfaction_solver_.state_.propagator_cardinality_
          .trigger_count_ != 0)
    std::cout << "count: "
              << solver.constrained_satisfaction_solver_.state_
                     .propagator_cardinality_.trigger_count_
              << std::endl;
  if (solver.constrained_satisfaction_solver_.state_.propagator_cardinality2_
          .trigger_count_ != 0) {
    int max_trigger = 0;
    for (auto c :
         solver.constrained_satisfaction_solver_.state_.propagator_cardinality2_
             .cardinality_database_.permanent_constraints_)
      max_trigger = std::max(max_trigger, c->trigger_count_);
    std::cout << "count2: "
              << solver.constrained_satisfaction_solver_.state_
                     .propagator_cardinality2_.trigger_count_
              << "  -  " << max_trigger << std::endl;
  }
  return solved_;
}
ProblemSpecification Pumpkin::ConvertProblem(sat::SatProblem &p) {
  if (add_encodings_) {
    if (cardinality_option_ == CardinalityOption::Sequential) {
      for (sat::CardinalityConstraint c : p.GetConstraints()) {
        assert(c.min == 0);
        p.AtMostK(c.max, c.lits);
      }
    } else {
      for (sat::CardinalityConstraint c : p.GetConstraints()) {
        sat::TotaliserEncoder::Encode(p, c.lits, c.min, c.max);
      }
    }
  }

  ProblemSpecification problem;
  problem.num_Boolean_variables_ = p.GetNrVars();
  for (const auto &c : p.GetClauses()) {
    std::vector<::Pumpkin::BooleanLiteral> clause;
    for (sat::Lit l : c) {
      ::Pumpkin::BooleanLiteral lit =
          ::Pumpkin::BooleanLiteral(BooleanVariable(l.x + 1), !l.complement);
      clause.push_back(lit);
    }
    problem.AddClause(clause);
  }

  for (sat::CardinalityConstraint c : p.GetConstraints()) {
    std::vector<::Pumpkin::BooleanLiteral> lits;
    for (sat::Lit l : c.lits) {
      ::Pumpkin::BooleanLiteral lit =
          ::Pumpkin::BooleanLiteral(BooleanVariable(l.x + 1), !l.complement);
      lits.push_back(lit);
    }
    if (!add_encodings_) {
      if (cardinality_option_ == CardinalityOption::Propagator) { //TODO less hacky way for this case
        IEncoder *encoder = nullptr;
        problem.propagator_cardinality_constraints_.push_back(
            ::Pumpkin::CardinalityConstraint(lits, c.min, c.max, encoder));
      } else {
        IEncoder *encoder;
        if (cardinality_option_ == CardinalityOption::Sequential)
          encoder = new ::Pumpkin::IncrementalSequentialEncoder(lits, c.min, c.max);
        else if (cardinality_option_ == CardinalityOption::Totolizer)
          encoder = new ::Pumpkin::TotaliserEncoder(lits, c.min, c.max);
        problem.dynamic_cardinality_constraints_.push_back(
            ::Pumpkin::CardinalityConstraint(lits, c.min, c.max, encoder));
      }
    }
  }
  for (auto l : p.GetMinimizeLit()) {
    problem.objective_literals_.push_back(WeightedLiteral(
        ::Pumpkin::BooleanLiteral(BooleanVariable(l.x + 1), true), 1));
  }

  return problem;
}
} // namespace simple_sat_solver::solver_wrappers
