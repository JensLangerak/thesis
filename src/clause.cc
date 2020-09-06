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

  //TODO
  if (lits_.size() == 1) {
    s->SetLitTrue(lits_[0], this);
  }

  return !lits_.empty();
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

  s->SetLitTrue(possibleUnit, this);
  return true;
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
Vec<Lit> Clause::CalcReason() {
  return lits_;
}
Vec<Lit> Clause::CalcReason(Lit p) {
  // TODO maybe check if clause had a reason for p
  Vec<Lit> reason;
  for (Lit l : lits_) {
    if (l.x == p.x)
      continue;
    else
      reason.push_back(l);
  }
  return reason;
}
}