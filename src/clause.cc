//
// Created by jens on 03-09-20.
//

#include <iostream>
#include <algorithm>
#include <stack>
#include "clause.h"
#include "solver.h"
namespace simple_sat_solver {
void Clause::Remove(simple_sat_solver::Solver *s) {
  s->RemoveFromWatchList(lits_[watchA_], this);
  if (watchA_ != watchB_) {
    s->RemoveFromWatchList(lits_[watchB_], this);
  }
//TODO
}
bool Clause::Simplify(simple_sat_solver::Solver *s) {
  if (lits_.size() == 1) {
    if (!s->SetLitTrue(lits_[0], this)) {
      return false;
    }
  }
  int index = -2;
  for (int i =0; i<lits_.size();i++) {
    if (s->GetLitValue(lits_[i]) == LBool::kTrue) {
      index = -1;
      break;
    }else if(s->GetLitValue(lits_[i]) == LBool::kUnknown) {
      if (index >= 0) {
        index = -1;
        break;
      } else {
        index =i;
      }
    }
  }

  if (index >= 0) {
    if (!s->SetLitTrue(lits_[index], this)) {
      return false;
    }
  }

  return index != -2;
}

void Clause::UndoUnitWatch(Solver *s) {
  if (watchA_ != watchB_) // not unit, thus ignore
    return;
  if (lits_.size() > 1)
    throw "NONON";
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
  if (!(lits_[watchA_] == p || lits_[watchB_] == p))
    throw "N";
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
    if (s->GetLitValue(lits_[watchA_]) == LBool::kTrue || s->GetLitValue(lits_[watchB_]) == LBool::kTrue) {
      s->AddWatch(p, this);
      return true;
    }
    // Lit is made false, search for a lit that is true or unknown.
    // Make sure that the lit is not watched by the other watcher.
    for (int i = 0; i < lits_.size(); i++) {
      if (i == watchA_ || i == watchB_)
        continue;
      if (s->GetLitValue(lits_[i]) != LBool::kFalse) {
        Lit lNew = lits_[i];
        if (p == lits_[watchA_]) {
          watchLast = watchA_;
          watchA_ = i;
        } else {
          watchLast = watchB_;
          watchB_ = i;
        }
        s->AddWatch(lNew, this);
        return true;
      }
    }

    activity_ += s->constrIncActivity;
    if (activity_ > 1e100)
      s->RescaleClauseActivity();
    // Could not update watcher. This means that the class is either Unit or False.
    if (watchA_ == watchB_) {
      s->AddWatch(p, this);
      return false;
    }
    /*
    //TODO do not update? Perhaps watchLast is then not needed?
    if (p == lits_[watchA_]) {
      watchLast = watchA_;
      watchA_ = watchB_;
    } else {
      watchLast = watchB_;
      watchB_ = watchA_;
    }*/
    // make sure that A is not false
    if(p == lits_[watchA_]) {
      Var temp = watchA_;
      watchA_ = watchB_;
      watchB_ = temp;
    }
    s->AddWatch(p, this);

    // For debug purposes, check if it is indeed unit.
    for (Lit l : lits_) {
      if (l == lits_[watchA_])
        continue;
      if (s->GetLitValue(l)!=LBool::kFalse)
        throw "Clause is not unit";
    }

    // Clause is unit, thus set Lit to true
    return s->SetLitTrue(lits_[watchA_], this);

  }
}
void Clause::Undo(Solver *s, Lit p) {
//TODO
}

Clause::Clause(Vec<Lit> ps, bool learnt, Solver &s) {
  learnt_ = learnt;
  watchLast = -1;
  activity_ = 1.0;
  locks_ = 0;

  std::sort (ps.begin(), ps.end(), [](Lit l, Lit r) { return l.x < r.x;});
  for (Lit l : ps) {
    if (lits_.empty() || lits_.back() != l) {
      lits_.push_back(l);
    } else {
      int seg = 2;
    }
  }

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
  Vec<Lit> cp = lits;
  //std::sort (lits.begin(), lits.end(), [](Lit l, Lit r) { return l.x < r.x;});
  for (Lit l : lits) {
    if (lits_.empty() || lits_.back() != l) {
      lits_.push_back(l);
    } else {
      int seg = 2;
    }
  }
  locks_ = 0;
  learnt_ = learnt;
  activity_ = 1.0;
  int indexUnit;
  for (int i = 0; i <lits_.size(); i++) {
    if (lits_[i] == unitLit) {
      indexUnit = i;
      break;
    }
  }
  watchA_ = indexUnit;
  watchB_ = mostRecentLearntIndex;
  watchLast = -1;
  // No watchb if there is only one literal
  if (watchB_ == -1) {
    if (lits.size() == 1)
      watchB_ = 0;
    else
      throw "Clause is not unit";
  }
  s.AddWatch(lits_[watchA_], this);
  s.AddWatch(lits_[watchB_], this);

}
void Clause::Lock() {
  if (locks_ != 0) {
    throw "WHAT!";
  }
  ++locks_;

}
void Clause::Unlock() {
  --locks_;
  if(locks_ != 0) {
    throw "WHAT!";
  }
}
bool Clause::Locked() {
  return locks_ > 0;
}
void Clause::RescaleActivity() {
  activity_ *= 1e-100;
}
bool Clause::Value(Solver *s) {
  for (Lit l : lits_)
    if (s->GetLitValue(l) == LBool::kTrue)
      return true;
  return false;
}
void Clause::CheckWatchers(Solver *s) {
  Lit a = lits_[watchA_];
  Lit b = lits_[watchB_];

  int ia = s->LitIndex(a);
  int ib = s->LitIndex(b);
  Vec<Constr*> wl = s->watches_[ia];
  bool wba = false;
  bool wbb = false;
  for (auto c : wl) {
    if(((Clause *) c)== this) {
      wba = true;
      break;
    }
  }
  wl = s->watches_[ib];
  for (auto c : wl) {
    if(((Clause *) c)== this) {
      wbb = true;
      break;
    }
  }

  if (!(wba && wbb)) {
    throw  "NOOOO";
  }
  if (s->GetLitValue(a) ==LBool::kTrue || s->GetLitValue(b) == LBool::kTrue)
    return;
  if (s->GetLitValue(a) == LBool::kFalse || s->GetLitValue(b) == LBool::kFalse)
  {
    std::queue<Lit> q = s->propagationQueue_;
    bool ba = s->GetLitValue(a) != LBool::kFalse;
    bool bb = s->GetLitValue(b) != LBool::kFalse;

    while(!q.empty()) {
      if (q.front() == a)
        ba = true;
      if (q.front() == b)
        bb = true;
      q.pop();
    }
    if (!(ba && bb))
      throw "waht";
  }
  if (watchA_ == watchB_) {
    if (s->GetLitValue(a) == LBool::kTrue)
      return;
    for(int i = 0; i < lits_.size(); i++) {
      if (i == watchA_ || i == watchB_)
        continue;

      throw "Error";
    }
  }

}
}