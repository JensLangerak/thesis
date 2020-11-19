//
// Created by jens on 18-09-20.
//

#include "simple_solver.h"
#include "../sat/constraints/cardinality_constraint.h"
#include "../sat/encoders/totaliser_encoder.h"
#include "../solver/solver.h"
#include <cassert>

namespace simple_sat_solver::solver_wrappers {

bool SimpleSolver::Solve(const sat::SatProblem &p2) {
  solved_ = false;
  sat::SatProblem p = p2;
  for (sat::IConstraint * c : p.GetConstraints()) {
    if (sat::CardinalityConstraint * car = dynamic_cast<sat::CardinalityConstraint*>(c)) {

      sat::TotaliserEncoder::Encode(p, car->lits, car->min, car->max);
    } else {
      assert(false); //Not implemented;
    }

  }
  solver::Solver s;
  for (int i = 0; i < p.GetNrVars(); i++)
    s.NewVar();
  for (const auto &c : p.GetClauses())
    s.AddClause(c);

  solved_ = s.Solve();
  if (solved_)
    solution_ = s.GetModel();

  return solved_;
}
std::vector<bool> SimpleSolver::GetSolution() const { return solution_; }
bool SimpleSolver::Optimize(const sat::SatProblem &p) { return false; }
} // namespace simple_sat_solver::solver_wrappers
