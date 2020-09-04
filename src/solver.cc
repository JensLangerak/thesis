//
// Created by jens on 03-09-20.
//

#include "solver.h"
#include "var_order.h"
namespace simple_sat_solver {
Solver::Solver() : order_(this->assigns_, this->activity_){

}

Var Solver::NewVar() {
  assigns_.push_back(LBool::kUnknown);
  return assigns_.size() - 1;
}
bool Solver::AddClause(Vec<Lit> literals) {
  Clause clause = Clause::NewClause(*this, literals, false);
  // TODO check if clause is true, empty, etc..
  constrs_.push_back(clause);
  return true;
}
bool Solver::SimplifyDb() {
  // TODO
  return false;
}
bool Solver::Solve(Vec<Lit> assumptions) {
  // TODO
  return false;
}

}