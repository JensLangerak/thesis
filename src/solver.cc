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
bool Solver::AddClause(const Vec<Lit> &literals) {
  Clause *c = new Clause(literals, false);
  constraints_.push_back(c);
  return true;
}
bool Solver::SimplifyDb() {
  // TODO
  return false;
}
bool Solver::Solve(const Vec<Lit> &assumptions) {
  // TODO
  return false;
}
Solver::~Solver() {
  for (auto c : constraints_)
    delete c;
  constraints_.clear();
}
const void Solver::PrintProblem() {
  std::cout << "Nr vars: " << varAssignments_.size() << std::endl;
  for(auto c : constraints_)
    c->PrintConstraint();
}
const void Solver::PrintAssinments() {
  for(int i = 0; i < varAssignments_.size(); i++) {
    LBool v = varAssignments_[i];
    std::cout << i << ": " << (v == LBool::kTrue ? "T" : v == LBool::kFalse ? "F" : "U" ) << std::endl;
  }

}
const void Solver::PrintFilledProblem() {
    std::cout << "Nr vars: " << varAssignments_.size() << std::endl;
    for(auto c : constraints_)
      c->PrintFilledConstraint(varAssignments_);
}

bool Solver::Solve() {
  for (auto c: constraints_) {
    if (!c->Simplify(this))
      return false;
  }
  bool stop = false;
  while (!stop) {
    if (!Propagate()) {
      //TODO conflict
      if (assumptions_.empty())
        return false;
      if (!Backtrack())
        return false;
    } else {
      stop = true;
      for (int i = 0; i < varAssignments_.size(); i++) {
        if (varAssignments_[i] == LBool::kUnknown) {
          stop = false;
          Lit assume;
          assume.x = i;
          assume.complement = false;
          Assume(assume);
          break;
        }
      }
    }
  }
    return true;
}
bool Solver::SetLitTrue(Lit lit) {
  LBool value = lit.complement ? LBool::kFalse : LBool::kTrue;
  Var x = lit.x;
  if (varAssignments_[x] == LBool::kUnknown) {
    varAssignments_[x] = value;
    propagationQueue_.push(~lit);
  } else if (varAssignments_[x] != value) {
    return false;
  }
  return true;
}
bool Solver::Propagate() {
  while(!propagationQueue_.empty()) {
    Lit lit = propagationQueue_.front();
    propagationQueue_.pop();
    for (auto c : constraints_) {
      if (!c->Propagate(this, lit))
        return false;
    }
  }
  return true;
}
const LBool Solver::GetLitValue(Lit l) {
  LBool var = varAssignments_[l.x];
  return l.complement ? ~var : var;
}
bool Solver::Backtrack() {
  while ((!assumptions_.empty()) && assumptions_.top().complement) {
    std::cout << "pop: " << assumptions_.top().x << std::endl;
    varAssignments_[assumptions_.top().x] = LBool::kUnknown;
    assumptions_.pop();
  }
  if (assumptions_.empty())
    return false;
  //TODO
  Lit assume = assumptions_.top();
  assumptions_.pop();
  varAssignments_[assume.x] = LBool::kUnknown;
  assume.complement = !assume.complement;
  Assume(assume);

  return true;
}
void Solver::Assume(Lit lit) {
  std::cout << "Assume: " << lit.x << (lit.complement ? "F" : "T") << std::endl;
  assumptions_.push(lit);
  SetLitTrue(lit);

}
bool Solver::AllAssigned() {
  for (LBool b : varAssignments_) {
    if (b == LBool::kUnknown)
      return false;
  }
  return true;
}

}