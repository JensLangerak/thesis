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
  if (lits_.size() == 1) {
    if (!s->SetLitTrue(lits_[0], this)) {
      return false;
    }
  }

  return !lits_.empty();
}

void Clause::UndoUnitWatch(Solver *s) {
  if (watchA_ != watchB_) // not unit, thus ignore
    return;
  if (watchLast == -1)
    throw "Cannot undo non updated watch.";
  watchB_ = watchLast;
  s->AddWatch(lits_[watchB_], this);
}

// Watchers are updated and unit is checked.
// We want to watch a literal that is true or unknown.
// If literal is true, we watch in case its value gets changed (after undo)
// If literal is unknown, if gets a value then either the clause becomes true
// or the clause might become unit. Or the clause becomes false.
// We will allways watch two different literals, unless there is only one literal not false.
bool Clause::Propagate(simple_sat_solver::Solver *s, simple_sat_solver::Lit p) {
  // If the lit makes the clause true, no need to update watches
  // thus only read the watcher
  if (s->GetLitValue(p) == LBool::kTrue) { //lit in clause is true
    if (p == lits_[watchA_] || p == lits_[watchB_]) {
      s->AddWatch(p, this);
      return true;
    } else {
      throw "Illegal state: notified by unregistered watch";
    }
  } else {
    // Lit is made false, search for a lit that is true or unknown.
    // Make sure that the lit is not watched by the other watcher.
    for (int i = 0; i < lits_.size(); i++) {
      if (i == watchA_ || i == watchB_)
        continue;
      if (s->GetLitValue(lits_[i]) != LBool::kFalse) {
        Lit lNew = lits_[i];
        s->AddWatch(lNew, this);
        if (p == lits_[watchA_]) {
          watchLast = watchA_;
          watchA_ = i;
        } else {
          watchLast = watchB_;
          watchB_ = i;
        }
        return true;
      }
    }
    // Could not update watcher. This means that the class is either Unit or False.
    if (watchA_ == watchB_)
      return false;
    //TODO do not update? Perhaps watchLast is then not needed?
    if (p == lits_[watchA_]) {
      watchLast = watchA_;
      watchA_ = watchB_;
    } else {
      watchLast = watchB_;
      watchB_ = watchA_;
    }

    // For debug purposes, check if it is indeed unit.
    for (Lit l : lits_) {
      if (l == lits_[watchA_])
        continue;
      if (s->GetLitValue(l)!=LBool::kFalse)
        throw "Clause is not unit";
    }

    // Clause is unit, thus set Lit to true
    s->SetLitTrue(lits_[watchA_], this);
    return true;

  }
}
void Clause::Undo(Solver *s, Lit p) {
//TODO
}

Clause::Clause(Vec<Lit> ps, bool learnt, Solver &s) {
  lits_ = ps;
  learnt_ = learnt;
  watchLast = -1;
  if (ps.size() == 0) {
    watchA_ = -1;
    watchB_ = -1;
  } else if (ps.size() == 1) {
    watchA_ = 0;
    watchB_ = 0;
    Lit l = lits_[0];
    s.AddWatch(l, this);
  } else {
    watchA_ = 0;
    watchB_ = 1;
    Lit lA = lits_[watchA_];
    Lit lB = lits_[watchB_];

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
  Vec<Lit> reason;
  for (Lit l : lits_) {
    if (l.x == p.x)
      continue;
    else
      reason.push_back(l);
  }
  return reason;
}
Clause::Clause(Vec<Lit> lits, bool learnt, Solver& s, Lit unitLit, int mostRecentLearntIndex) {
  // assert(lits != {lits[0], ~list[1]})
  lits_ = lits;
  learnt_ = learnt;
  int indexUnit;
  for (int i = 0; i <lits_.size(); i++) {
    if (lits_[i] == unitLit) {
      indexUnit = i;
      break;
    }
  }
  watchA_ = indexUnit;
  watchB_ = indexUnit;
  watchLast = mostRecentLearntIndex;
  // No watchLast if there is only one literal
  if (watchLast == -1) {
    if (lits.size() == 1)
      watchLast = 0;
    else
      throw "Clause is not unit";
  }
  s.AddWatch(lits_[watchA_], this);

}
}