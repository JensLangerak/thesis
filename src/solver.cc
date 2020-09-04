//
// Created by jens on 03-09-20.
//

#include <iostream>
#include "solver.h"
#include "var_order.h"
namespace simple_sat_solver {
Solver::Solver() {

}

Var Solver::NewVar() {
  varAssignments_.push_back(LBool::kUnknown);
  return varAssignments_.size() - 1;
}
bool Solver::AddClause(Vec<Lit> literals) {
  Clause *c = new Clause(literals, false);
  constraints_.push_back(c);
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
Solver::~Solver() {
  for (auto c : constraints_)
    delete c;
  constraints_.clear();
}
void Solver::PrintProblem() {
  std::cout << "Nr vars: " << varAssignments_.size() << std::endl;
  for(auto c : constraints_)
    c->PrintConstraint();
}
void Solver::PrintAssinments() {
  for(int i = 0; i < varAssignments_.size(); i++) {
    LBool v = varAssignments_[i];
    std::cout << i << ": " << (v == LBool::kTrue ? "T" : v == LBool::kFalse ? "F" : "U" ) << std::endl;
  }

}
void Solver::PrintFilledProblem() {
    std::cout << "Nr vars: " << varAssignments_.size() << std::endl;
    for(auto c : constraints_)
      c->PrintFilledConstraint(varAssignments_);
}

}