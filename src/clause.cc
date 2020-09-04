//
// Created by jens on 03-09-20.
//

#include <iostream>
#include "clause.h"
namespace simple_sat_solver {
bool Clause::Locked(simple_sat_solver::Solver *s) {
  return false; //TODO
}
void Clause::Remove(simple_sat_solver::Solver *s) {
//TODO
}
bool Clause::Simplify(simple_sat_solver::Solver *S) {
  return false;//TODO
}
bool Clause::Propagate(simple_sat_solver::Solver *S, simple_sat_solver::Lit p) {
  return false;//TODO
}
void Clause::CalcReason(simple_sat_solver::Solver *S,
                                           simple_sat_solver::Lit p,
                                           simple_sat_solver::Vec<simple_sat_solver::Lit> out_reason) {
//TODO
}
void Clause::Undo(Solver *s, Lit p) {
//TODO
}
Clause::Clause(Vec<Lit> ps, bool learnt) {
  lits_ = ps;
  learnt_ = learnt;
}
Clause::~Clause() {

}
void Clause::PrintConstraint() {
  std::cout << "Clause: ";
  for (auto l: lits_) {
    if (l.complement)
      std::cout << "~";

    std::cout << l.x << " ";
  }
  std::cout << std::endl;
}
void Clause::PrintFilledConstraint(const Vec<LBool> &vars) {
  std::cout << "Clause: ";
  for (auto l: lits_) {
    LBool lb = vars[l.x];
    if (lb == LBool::kUnknown) {
      if (l.complement)
        std::cout << "~";
      std::cout << l.x << " ";
    } else {
      bool b = lb == LBool::kTrue;
      b = l.complement ? !b : b;
      std::cout << (b? "T" : "F") << " ";
    }
  }
  std::cout << std::endl;
}
}