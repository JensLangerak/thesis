//
// Created by jens on 15-10-20.
//

#include "pumpkin.h"


#include "../pumpkin/Basic Data Structures/boolean_literal.h"
#include "../pumpkin/Engine/constraint_optimisation_solver.h"
#include "../pumpkin/Basic Data Structures/solver_parameters.h"

namespace simple_sat_solver::solver_wrappers {
using namespace Pumpkin;

bool Pumpkin::Solve(const sat::SatProblem &p) {
  solved_ = false;
  ProblemSpecification problem;
  problem.num_Boolean_variables_ = p.GetNrVars();

  for (const auto &c : p.GetClauses()) {
    std::vector<::Pumpkin::BooleanLiteral> clause;
    for (sat::Lit l : c) {
      ::Pumpkin::BooleanLiteral lit = ::Pumpkin::BooleanLiteral(BooleanVariable(l.x + 1), !l.complement);
      clause.push_back(lit);
    }
    problem.AddClause(clause);

  }

  SolverParameters params;
  params.bump_decision_variables = true;
  ConstraintSatisfactionSolver solver(problem, params);
  SolverOutput solver_output = solver.Solve(std::numeric_limits<double>::max());
  solved_ = solver_output.HasSolution();

  assert(solver_output.solution.size() - 1 == p.GetNrVars());
  if (solved_) {
    solution_ = std::vector<bool>();
    for (int i = 1; i < solver_output.solution.size(); ++i)
      solution_.push_back(solver_output.solution[i]);
  }

  return solved_;
}
std::vector<bool> Pumpkin::GetSolution() const { return solution_; }
} // namespace simple_sat_solver::solver_wrappers
