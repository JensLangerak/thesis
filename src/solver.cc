//
// Created by jens on 03-09-20.
//

#include "solver.h"
#include "var_order.h"
namespace simple_sat_solver {
Solver::Solver() {

}

Var Solver::NewVar() {
  return -1;
}
bool Solver::AddClause(Vec<Lit> literals) {
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