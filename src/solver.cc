//
// Created by jens on 03-09-20.
//

#include "solver.h"

#include <bits/stdc++.h>
#include <iostream>

#include "var_order.h"

namespace simple_sat_solver {
Solver::Solver() : constrIncActivity_(1.0), constrDecayFactor_(1.0 / 0.95) {}

Var Solver::NewVar() {
  varAssignments_.push_back(LBool::kUnknown);
  reason_.push_back(nullptr);
  level_.push_back(-1);
  // watch list for true and false lit.
  watches_.push_back(Vec<Clause *>());
  watches_.push_back(Vec<Clause *>());

  varOrder.NewVar();
  return varAssignments_.size() - 1;
}

bool Solver::AddClause(const Vec<Lit> &literals) {
  // TODO return false if c is empty and ignore when contains both x and -x
  Clause *c = new Clause(literals, false, *this);
  constraints_.push_back(c);
  return true;
}

Solver::~Solver() {
  for (auto c : constraints_)
    delete c;
  for (auto c : learntClauses_)
    delete c;
  constraints_.clear();
}

void Solver::PrintProblem() {
  std::cout << "Nr vars: " << varAssignments_.size() << std::endl;
  for (auto c : constraints_)
    c->PrintConstraint();
}

void Solver::PrintAssignments() {
  for (int i = 0; i < varAssignments_.size(); i++) {
    LBool v = varAssignments_[i];
    std::cout << i << ": "
              << (v == LBool::kTrue    ? "T"
                  : v == LBool::kFalse ? "F"
                                       : "U")
              << std::endl;
  }
}

void Solver::PrintFilledProblem() {
  std::cout << "Nr vars: " << varAssignments_.size() << std::endl;
  for (auto c : constraints_)
    c->PrintFilledConstraint(varAssignments_);
}

bool Solver::Solve() {
  int maxLearnt = 20;
  LBool res = LBool::kUnknown;
  while (res == LBool::kUnknown) {
    maxLearnt *= 1.1;
    res = Solve(maxLearnt);
  }
  return res == LBool::kTrue;
}

LBool Solver::Solve(int maxLearnt) {
  for (auto c : constraints_) {
    if (!c->Simplify(*this))
      return LBool::kFalse;
  }
  for (auto c : learntClauses_) {
    if (!c->Simplify(*this))
      return LBool::kFalse;
  }
  bool stop = false;
  while (!stop) {
    Clause * conflict;
    if (!Propagate(conflict)) { // conflict found
      while (!propagationQueue_.empty())
        propagationQueue_.pop();
      if (!HandleConflict(conflict))
        return LBool::kFalse;
    } else {
      if (learntClauses_.size() > maxLearnt) {
        ReduceDB(maxLearnt);
        Backtrack(0);
        while (!propagationQueue_.empty())
          propagationQueue_.pop();
        return LBool::kUnknown; // TODO not return
      }
      stop = !AddAssumption();
      constrIncActivity_ *= constrDecayFactor_;
    }
  }

  return AllAssigned() ? CheckConstraints() : LBool::kUnknown;
}

bool Solver::SetLitTrue(Lit lit, Clause *constr) {
  LBool value = lit.complement ? LBool::kFalse : LBool::kTrue;
  Var x = lit.x;
  if (varAssignments_[x] == LBool::kUnknown) {
    varAssignments_[x] = value;
    propagationQueue_.push(~lit);
    learnt_.push(lit);
    level_[x] = (decisionLevels_.empty() ? 0 : decisionLevels_.top());
    reason_[x] = constr;
    if (constr != nullptr)
      constr->Lock();
  } else if (varAssignments_[x] != value) {
    return false;
  }
  return true;
}

bool Solver::Propagate(Clause *&conflict) {
  while (!propagationQueue_.empty()) {
    Lit lit = propagationQueue_.front();
    propagationQueue_.pop();
    int index = LitIndex(lit);
    // clear original watch list, clauses should re-add themselves to the list.
    Vec<Clause *> watchList = watches_[index];
    watches_[index].clear();

    for (int i = 0; i < watchList.size(); i++) {
      auto c = watchList[i];
      if (GetLitValue(lit) == LBool::kUnknown)
        throw "Should not propagate unknown literals";
      if (!c->Propagate(*this, lit)) {
        // conflict found
        conflict = c;
        // re-add the unhandled watches, excluding the current one.
        for (i = i + 1; i < watchList.size(); i++)
          watches_[index].push_back(watchList[i]);
        return false;
      }
    }
  }
  return true;
}

LBool Solver::GetLitValue(Lit l) const {
  LBool var = varAssignments_[l.x];
  return l.complement ? ~var : var;
}

bool Solver::UndoOne() {
  if (learnt_.empty())
    return false;

  Lit l = learnt_.top();
  learnt_.pop();
  varAssignments_[l.x] = LBool::kUnknown;
  varOrder.Undo(l.x);

  level_[l.x] = -1;
  if (reason_[l.x] != nullptr)
    reason_[l.x]->Unlock();
  reason_[l.x] = nullptr;

  return true;
}

void Solver::UndoDecisions(int level) {
  while (learnt_.size() >= level) {
    UndoOne();
  }
}

bool Solver::Backtrack(int level) {
  while (!decisionLevels_.empty() && decisionLevels_.top() > level) {
    UndoDecisions(decisionLevels_.top());
    decisionLevels_.pop();
  }
  return true;
}

void Solver::Assume(Lit lit) {
  decisionLevels_.push(learnt_.size() + 1);
  SetLitTrue(lit, nullptr);
}

bool Solver::AllAssigned() const {
  for (LBool b : varAssignments_) {
    if (b == LBool::kUnknown)
      return false;
  }
  return true;
}

Vec<Lit> Solver::Analyze(const Clause *constr) {
  Vec<Lit> learnt;
  Vec<bool> seen(varAssignments_.size());
  Vec<Lit> conflictReason = constr->CalcReason();
  int levelCount = 0; // Keeps track of vars in the current decision level.
  do {
    // handle current conflict reasons
    for (Lit l : conflictReason) {
      if (seen[l.x])
        continue;
      seen[l.x] = true;
      varOrder.Update(l.x);
      if (level_[l.x] == decisionLevels_.top()) {
        ++levelCount;
      } else {
        learnt.push_back(l);
      }
    }

    // go back in time, doing so will reveal an uip
    Lit p;
    Clause *r;
    do {
      p = learnt_.top();
      r = reason_[p.x];
      UndoOne();
    } while (!seen[p.x]);
    if (levelCount <= 1) { // UIP found
      learnt.push_back(~p);
      return learnt;
    }
    conflictReason = r->CalcReason(p);
    --levelCount;
  } while (levelCount > 0); // UIP found

  return learnt;
}

void Solver::AddWatch(Lit &lit, Clause *clause) {
  watches_[LitIndex(lit)].push_back(clause);
}

int Solver::LitIndex(Lit &lit) { return lit.x * 2 + (lit.complement ? 1 : 0); }
Lit Solver::GetMostRecentLit(Vec<Lit> lits) {
  // values that are popped from learnt are temporarily stored in hist.
  std::stack<Lit> hist;
  Lit last;
  last.x = -1;
  while (!learnt_.empty()) {
    for (auto & lit : lits) {
      if (lit.x == learnt_.top().x) {
        last = lit;
        break;
      }
    }
    if (last.x != -1)
      break;

    hist.push(learnt_.top());
    learnt_.pop();
  }

  // push the popped values back on learnt
  while (!hist.empty()) {
    learnt_.push(hist.top());
    hist.pop();
  };
  return last;
}

bool Solver::HandleConflict(const Clause * conflict) {
  if (decisionLevels_.empty()) // cannot backtrack, so un sat
    return false;

  std::stack<Lit> hist = learnt_;
  std::stack<int> highestLevel = decisionLevels_;
  Vec<Lit> c = Analyze(conflict);
  varOrder.UpdateAll();

  // Find the clause that will be unit and find the lowest backtrackLevel
  int backtrackLevel = -1;
  Lit unit;
  for (Lit l : c) {
    int level = level_[l.x];
    if (level != -1) {
      backtrackLevel = std::max(backtrackLevel, level);
    } else {
      unit = l;
    }
  }

  if (!Backtrack(backtrackLevel)) // cannot backtrack, so it is unsatisfiable
    return false;

  Lit mostRecentLit = GetMostRecentLit(c); // needed for the watchers
  Clause *clause = new Clause(c, true, *this, unit, mostRecentLit);

  // Added clause is unit, so set lit
  learntClauses_.push_back(clause);
  SetLitTrue(unit, clause);
  return true;
}

bool Solver::AddAssumption() {
  Var v = varOrder.Select(varAssignments_);
  if (v < 0)
    return false;
  Assume(Lit(v, false));
  return true;
}

void Solver::RescaleClauseActivity() {
  for (Clause *c : learntClauses_)
    c->RescaleActivity();
  constrIncActivity_ *= 1e-100;
}

void Solver::ReduceDB(int learnt) {
  std::priority_queue<Clause *> queue;
  for (Clause *c : learntClauses_) {
    queue.push(c);
  }
  while (!learntClauses_.empty())
    learntClauses_.pop_back();
  for (int i = 0; i < learntClauses_.size(); i++) {
    learntClauses_.push_back(queue.top());
    queue.pop();
  }
  while (!queue.empty()) {
    Clause *c = queue.top();
    queue.pop();
    if (c->Locked()) {
      learntClauses_.push_back(c);
    } else {
      c->Remove(*this);
      delete c;
    }
  }
}

void Solver::RemoveFromWatchList(Lit &lit, const Clause *clause) {
  int index = this->LitIndex(lit);
  watches_[index].erase(
      std::remove(watches_[index].begin(), watches_[index].end(), clause),
      watches_[index].end());
  for (auto c : watches_[index]) {
    if (c == clause)
      throw "ERROR";
  }
}

LBool Solver::CheckConstraints() {
  for (auto c : constraints_) {
    if (!c->Value(*this)) {
      throw "Should not happen";
    }
  }
  return LBool::kTrue;
}

void Solver::CheckWatchers() {
  for (auto c : constraints_) {
    c->CheckWatchers(this);
  }
  for (auto c : learntClauses_) {
    c->CheckWatchers(this);
  }
}
Vec<bool> Solver::GetModel() const {
  Vec<bool> res(varAssignments_.size());
  for (int i = 0; i < varAssignments_.size(); i++) {
    switch (varAssignments_[i]) {
    case LBool::kUnknown:
      throw "Problem is not solved";
    case LBool::kTrue:
      res[i] = true;
      break;
    case LBool::kFalse:
      res[i] = false;
      break;
    }
  }
  return simple_sat_solver::Vec<bool>();
}
} // namespace simple_sat_solver
