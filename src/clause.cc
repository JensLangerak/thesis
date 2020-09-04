//
// Created by jens on 03-09-20.
//

#include <iostream>
#include <algorithm>
#include <stack>
#include "clause.h"
#include "solver.h"
namespace simple_sat_solver {
bool Clause::Locked(simple_sat_solver::Solver *s) {
  return false; //TODO
}
void Clause::Remove(simple_sat_solver::Solver *s) {
//TODO
}
bool Clause::Simplify(simple_sat_solver::Solver *s) {
  std::sort(lits_.begin(), lits_.end(), [](const Lit &p, const Lit &q) -> bool {
    return p.x == q.x ? p.complement < q.complement : p.x < q.x;
  });

  //TODO should be a better way to do this
  std::stack<int> removeVars;
  for (int i = 0; i < lits_.size() - 1; i++) {
    if (lits_[i].x == lits_[i+1].x) {
      removeVars.push(i);
      removeVars.push(i + 1);
      ++i;
    }
  }
  while(!removeVars.empty()) {
    int index = removeVars.top();
    removeVars.pop();
    lits_.erase(lits_.begin() + index);
  }

  if (lits_.size() == 1) {
    s->SetLitTrue(lits_[0]);
  }

  return !lits_.empty();//TODO
}
bool Clause::Propagate(simple_sat_solver::Solver *s, simple_sat_solver::Lit p) {
  // TODO keep track of the assigned values
  Lit possibleUnit;
  possibleUnit.x = -1;
  for (auto lit : lits_) {
    if (s->GetLitValue(lit) == LBool::kTrue)
      return true;
    else if (s->GetLitValue(lit) == LBool::kUnknown) {
      if (possibleUnit.x >= 0 )
        return true;
      possibleUnit = lit;
    }

  }
  if (possibleUnit.x < 0)
    return false;

  s->SetLitTrue(possibleUnit);
  return true;
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
const void Clause::PrintConstraint() {
  std::cout << "Clause: ";
  for (auto l: lits_) {
    if (l.complement)
      std::cout << "~";

    std::cout << l.x << " ";
  }
  std::cout << std::endl;
}
const void Clause::PrintFilledConstraint(const Vec<LBool> &vars) {
  std::cout << "Clause: ";
  for (auto l: lits_) {
    LBool lb = vars[l.x];
      if (l.complement)
        std::cout << "~";
      std::cout << l.x;
    if (lb != LBool::kUnknown) {
      bool b = lb == LBool::kTrue;
      b = l.complement ? !b : b;
      std::cout << (b? "T" : "F") ;
    }
    std::cout << " ";
  }
  std::cout << std::endl;
}
}