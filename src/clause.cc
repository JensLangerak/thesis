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

void Clause::UpdateWatches(Solver *s) {
  if (watchA != watchB)
    return;
  if (watchLast == -1)
    throw "erro";
  watchB = watchLast;
  s->AddWatch(lits_[watchB], this);
  /*for (int i =0; i <lits_.size(); i++) {
    if (i == watchA)
      continue;
    if (s->GetLitValue(lits_[i]) == LBool::kUnknown) {
      watchB =i;
      Lit watch = lits_[watchB];
      s->AddWatch(watch, this);
      return;
    }
  }
  throw "error";
*/
}

bool Clause::Propagate(simple_sat_solver::Solver *s, simple_sat_solver::Lit p) {
  // TODO keep track of the assigned values
  if (s->GetLitValue(p) == LBool::kTrue) { //lit in clause is true
    if (p == lits_[watchA] || p == lits_[watchB]) {
      s->AddWatch(p, this);
      return true;
    } else {
      throw "ERROR";
    }
  } else {
    for (int i = 0; i < lits_.size(); i++) {
      if (i == watchA || i == watchB)
        continue;
      if (s->GetLitValue(lits_[i]) != LBool::kFalse) {
        Lit lNew = lits_[i];
        s->AddWatch(lNew, this);
        if (p == lits_[watchA]) {
          watchLast = watchA;
          watchA = i;
        } else {
          watchLast = watchB;
          watchB = i;
        }
        return true;
      }
    }
    if (watchA == watchB)
      return false;
    if (p == lits_[watchA]) {
      watchLast = watchA;
      watchA = watchB;
    } else {
      watchLast = watchB;
      watchB = watchA;
    }
    //TODO DEBUG check
    for (Lit l : lits_) {
      if (l == lits_[watchA])
        continue;
      if (s->GetLitValue(l)!=LBool::kFalse)
        throw "Error";
    }
    s->SetLitTrue(lits_[watchA], this);
    return true;

  }
  
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

Clause::Clause(Vec<Lit> ps, bool learnt, Solver &s) {
  lits_ = ps;
  learnt_ = learnt;
  watchLast = -1;
  if (ps.size() == 0) {
    watchA = -1;
    watchB = -1;
  } else if (ps.size() == 1) {
    watchA = 0;
    watchB = 0;
    Lit l = lits_[0];
    s.AddWatch(l, this);
  } else {
    watchA = 0;
    watchB = 1;
    Lit lA = lits_[watchA];
    Lit lB = lits_[watchB];

    s.AddWatch(lA, this);
    s.AddWatch(lB, this);
  }
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
Clause::Clause(Vec<Lit> lits, bool learnt, Solver& s, Lit watchLit) {
  lits_ = lits;
  learnt_ = learnt;
  int indexUnit;
  int indexLast = -1;
  for (int i = 0; i <lits_.size(); i++) {
    if (lits_[i] == watchLit) {
      indexUnit = i;
      break;
    }
  }
  std::stack<Lit> hist;
  bool done = false;
  while(!s.learnt_.empty()) {
    for (int i = 0; i < lits_.size(); i++) {
      if (lits[i].x == s.learnt_.top().x) {
        indexLast = i;
        done = true;
      }
    }
    if (done)
      break;

    hist.push(s.learnt_.top());
    s.learnt_.pop();
  }
  while (!hist.empty()) {
    s.learnt_.push(hist.top());
    hist.pop();
  }

  watchA = indexUnit;
  watchB = indexUnit;
  watchLast = indexLast;
  if (watchLast == -1) { // TODO check what this case is
    if (lits_.size() > 2)
      throw "error";
    if (lits_.size() == 1)
      watchLast = 0;
    else
      watchLast = 1- indexUnit; //1 or 0
  }
  s.AddWatch(lits_[watchA], this);

}
}