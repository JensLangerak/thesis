//
// Created by jens on 15-10-20.
//

#include "pumpkin.h"

#include "../pumpkin/Basic Data Structures/boolean_literal.h"
#include "../pumpkin/Basic Data Structures/problem_specification.h"
#include "../pumpkin/Basic Data Structures/solver_parameters.h"
#include "../pumpkin/Engine/constraint_optimisation_solver.h"
#include "../sat/encoders/totaliser_encoder.h"

namespace simple_sat_solver::solver_wrappers {
using namespace Pumpkin;

bool Pumpkin::Solve(const sat::SatProblem &p2) {
  solved_ = false;
  sat::SatProblem p = p2;
//  sat::SatProblem p3 = p2;
//    for (sat::CardinalityConstraint c : p.GetConstraints()) {
//      sat::TotaliserEncoder::Encode(p3, c.lits, c.min, c.max);
//    }

//  ProblemSpecification problem2;
//  problem2.num_Boolean_variables_ = p3.GetNrVars();
//  for (const auto &c : p3.GetClauses()) {
//    std::vector<::Pumpkin::BooleanLiteral> clause;
//    for (sat::Lit l : c) {
//      ::Pumpkin::BooleanLiteral lit =
//          ::Pumpkin::BooleanLiteral(BooleanVariable(l.x + 1), !l.complement);
//      clause.push_back(lit);
//    }
//    problem2.AddClause(clause);
//  }

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
    problem.cardinality_constraints_.push_back(
        ::Pumpkin::CardinalityConstraint(lits, c.min, c.max));
  }

  SolverParameters params;
  params.bump_decision_variables = true;
  ConstraintSatisfactionSolver solver(problem, params);
//  ConstraintSatisfactionSolver solver2(problem2, params);
//  assert(solver.state_.GetNumberOfVariables() == solver2.state_.GetNumberOfVariables());
//  assert(solver.state_.propagator_clausal_.clause_database_.permanent_clauses_.size() == solver2.state_.propagator_clausal_.clause_database_.permanent_clauses_.size());
//  assert(solver.state_.propagator_clausal_.clause_database_.unit_clauses_.size() == solver2.state_.propagator_clausal_.clause_database_.unit_clauses_.size());
//
//  for (int i = 0; i < solver.state_.propagator_clausal_.clause_database_.unit_clauses_.size(); ++i) {
//    assert(solver.state_.propagator_clausal_.clause_database_.unit_clauses_[i] == solver2.state_.propagator_clausal_.clause_database_.unit_clauses_[i]);
//  }
//  for (int i = 0; i < solver.state_.propagator_clausal_.clause_database_.permanent_clauses_.size(); ++i) {
//    auto w1 =solver.state_.propagator_clausal_.clause_database_.permanent_clauses_[i];
//    auto w2 = solver2.state_.propagator_clausal_.clause_database_.permanent_clauses_[i];
//    std::vector<::Pumpkin::BooleanLiteral> l1;
//    std::vector<::Pumpkin::BooleanLiteral> l2;
//    for (int i = 0; i < w1->Size(); ++i) {
//      l1.push_back(w1->literals_[i]);
//      l2.push_back(w2->literals_[i]);
//      assert(w1->literals_[i] == w2->literals_[i]);
//    }
////    assert(w1->literals_ == w2->literals_);
//  }
  SolverOutput solver_output = solver.Solve(std::numeric_limits<double>::max());
  solved_ = solver_output.HasSolution();

  assert(solver_output.solution.size() - 1 >= p.GetNrVars());
  if (solved_) {
    solution_ = std::vector<bool>();
    for (int i = 1; i < solver_output.solution.size(); ++i)
      solution_.push_back(solver_output.solution[i]);
  }

  return solved_;
}
std::vector<bool> Pumpkin::GetSolution() const { return solution_; }
} // namespace simple_sat_solver::solver_wrappers
