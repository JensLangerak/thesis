//
// Created by jens on 03-09-20.
//

#include <iostream>
#include "solver.h"
#include "var_order.h"
#include <bits/stdc++.h>
namespace simple_sat_solver {
Solver::Solver() {
  constrIncActivity = 1.0;
  constrDecayFactor = 1.0 / 0.95;
}

Var Solver::NewVar() {
  varAssignments_.push_back(LBool::kUnknown);
  reason_.push_back(nullptr);
  level_.push_back(-1);
  watches_.push_back(Vec<Clause*>());
  watches_.push_back(Vec<Clause*>());
  varOrder.NewVar();
  return varAssignments_.size() - 1;
}
bool Solver::AddClause(const Vec<Lit> &literals) {
  Clause *c = new Clause(literals, false, *this);
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
  for (auto c: learntClauses_)
    delete c;
  constraints_.clear();
}
void Solver::PrintProblem() {
  std::cout << "Nr vars: " << varAssignments_.size() << std::endl;
  for(auto c : constraints_)
    c->PrintConstraint();
}
void Solver::PrintAssignments() {
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

bool Solver::Solve() {
  int maxLearnt = 20;
  LBool res = LBool::kUnknown;
  while(res == LBool::kUnknown) {
    maxLearnt *= 1.1;
    res = Solve(maxLearnt);
    //CheckWatchers();
  }
  return res == LBool::kTrue;
}
LBool Solver::Solve(int maxLearnt) {
  for (auto c: constraints_) {
    if (!c->Simplify(*this))
      return LBool::kFalse;
  }
  for (auto c: learntClauses_) {
    if (!c->Simplify(*this))
      return LBool::kFalse;
  }
  bool stop = false;
  while (!stop) {
    if (!Propagate()) { // conflict found
      while(!propagationQueue_.empty())
        propagationQueue_.pop();
      if (!HandleConflict())
        return LBool::kFalse;
    } else {
      if (learntClauses_.size() > maxLearnt) {
        //CheckWatchers();
        ReduceDB(maxLearnt);
        //CheckWatchers();
        Backtrack(0);
        //CheckWatchers();
        while (!propagationQueue_.empty())
          propagationQueue_.pop();
        return LBool::kUnknown; //TODO not return
      }
      if (!propagationQueue_.empty()) {
        int ikSnapHetNiet = 2;
      }
      //CheckWatchers();
      stop = !AddAssumption();
      constrIncActivity *= constrDecayFactor;
    }


  }

  if (AllAssigned() && (CheckConstraints() == LBool::kFalse)) {
    int faesg = 325;
  }
  return AllAssigned() ? CheckConstraints() : LBool::kUnknown;
}
bool Solver::SetLitTrue(Lit lit, Clause * constr) {
  LBool value = lit.complement ? LBool::kFalse : LBool::kTrue;
  Var x = lit.x;
  if (varAssignments_[x] == LBool::kUnknown) {
    //std::cout << "Set " << lit.x << " " << (lit.complement ? "F" : "T") << std::endl;
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
bool Solver::Propagate() {
  while(!propagationQueue_.empty()) {
    //CheckWatchers();
    Lit lit = propagationQueue_.front();
    propagationQueue_.pop();
    //std::cout << "Propagat: " << lit.x << " " << (lit.complement ? "F" : "T") << std::endl;
    int index = LitIndex(lit);
    // clear original watch list, clauses should re add themselves to the list.
    Vec<Clause *> watchList = watches_[index];
    watches_[index].clear();

    for (int i = 0; i < watchList.size(); i++) {
      auto c = watchList[i];
      if (GetLitValue(lit) == LBool::kUnknown)
        throw "Should not propagate unknown literals";
      if (!c->Propagate(*this, lit)){
        // conflict found
        this->conflictReason_ = c;
        // re-add the unhandled watches, excluding the current one.
        for (i = i+1; i < watchList.size(); i++)
          watches_[index].push_back(watchList[i]);
        return false;
      }
    }
  }

  //CheckWatchers();
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
  //std::cout << "Undo: " << l.x << std::endl;
  level_[l.x] = -1;
  if (reason_[l.x] != nullptr)
    reason_[l.x]->Unlock();
  reason_[l.x] = nullptr;

  // Update the unit watches that might be no longer unit
  Lit complL = ~l;
  return true;
}

void Solver::UndoDecisions(int level) {
  while (learnt_.size() >= level) {
    UndoOne();
  }
}
bool Solver::Backtrack(int level) {
  while (!decisionLevels_.empty() && decisionLevels_.top() > level) {
    //std::cout << "Pop: " << decisionLevels_.top() << std::endl;
    UndoDecisions(decisionLevels_.top());
    decisionLevels_.pop();
  }
  return true;
}

void Solver::Assume(Lit lit) {
 //std::cout <<learnt_.size() <<  " Assume: " << lit.x << (lit.complement ? "F" : "T") << std::endl;
 // TODO decisionLevels 0 -1
  decisionLevels_.push(learnt_.size() + 1);
  SetLitTrue(lit, nullptr);
}
bool Solver::AllAssigned() {
  for (LBool b : varAssignments_) {
    if (b == LBool::kUnknown)
      return false;
  }
  return true;
}
Vec<Lit> Solver::Analyze(Clause *constr) {
  Vec<Lit> learnt;
  Vec<bool> seen(varAssignments_.size());
  Vec<Lit> conflictReason = conflictReason_->CalcReason();
  int level_count = 0;
  do {
    // handle current conflict reasons
    for(Lit l : conflictReason) {
      if (seen[l.x])
        continue;
      seen[l.x] = true;
      varOrder.Update(l.x);
      if (level_[l.x] == decisionLevels_.top()) {
        ++level_count;
      } else {
        learnt.push_back(l);
      }
    }

    // go back in time, doing so will reveal an uip
    Lit p;
    Clause* r;
    do {
      p = learnt_.top();
      r = reason_[p.x];
      UndoOne();
    } while(!seen[p.x]);
    if (level_count <= 1) { // UIP found
      learnt.push_back(~p);
      return learnt;
    }
    conflictReason = r->CalcReason(p);
    --level_count;
  } while(level_count > 0);//UIP found

  return learnt;
}
void Solver::AddWatch(Lit &lit, Clause *p_clause) {
  watches_[LitIndex(lit)].push_back(p_clause);
}

int Solver::LitIndex(Lit &lit) {
  return lit.x * 2 + (lit.complement ? 1 : 0);
}
Lit Solver::GetMostRecentLit(Vec<Lit> lits) {
  std::stack<Lit> hist;
  Lit last;
  last.x = -1;
  while(!learnt_.empty()) {
    for (int i = 0; i < lits.size(); i++) {
      if (lits[i].x == learnt_.top().x) {
        last = lits[i];
        break;
      }
    }
    if (last.x != -1)
      break;

    hist.push(learnt_.top());
    learnt_.pop();
  }
  while (!hist.empty()) {
    learnt_.push(hist.top());
    hist.pop();
  };
  return last;
}
bool Solver::HandleConflict() {
  if (decisionLevels_.empty()) // cannot backtrack, so un sat
    return false;

  std::stack<Lit> hist = learnt_;
  std::stack<int> highestLevel = decisionLevels_;
  Vec<Lit> c = Analyze(conflictReason_);
  varOrder.UpdateAll();

  // Find the clause that will be unit and find the lowest backtrackLevel
  int backtrackLevel = -1;
  Lit unit;
  for(Lit l : c) {
    int level = level_[l.x];
    if (level != -1) {
      backtrackLevel = std::max(backtrackLevel, level);
    } else {
      unit = l;
    }
  }

  if (!Backtrack(backtrackLevel)) // cannot backtrack, so it is unsatisfiable
    return false;

  //CheckWatchers();
  Lit mostRecentLit = GetMostRecentLit(c); // needed for the watchers
  Clause *clause = new Clause(c, true, *this, unit, mostRecentLit);

  // Added clause is unit, so set lit
  learntClauses_.push_back(clause);
  SetLitTrue(unit, clause);
  //clause->CheckWatchers(this);
  //CheckWatchers();
  return true;
}
bool Solver::AddAssumption() {
  Var v = varOrder.Select(varAssignments_);
  if (v < 0)
    return false;
  Lit assume;
  assume.x = v;
  assume.complement = false;
  Assume(assume);
  return true;
/*
  for (int i = 0; i < varAssignments_.size(); i++) {
    if (varAssignments_[i] == LBool::kUnknown) {
      Lit assume;
      assume.x = i;
      assume.complement = false;
      Assume(assume);
      return true;
    }
  }
  return false; */
}
void Solver::RescaleClauseActivity() {
  for (Clause *c : learntClauses_)
    c->RescaleActivity();
  constrIncActivity *= 1e-100;

}
void Solver::ReduceDB(int learnt) {
  std::priority_queue<Clause *> queue;
  for (Clause * c : learntClauses_) {
    queue.push(c);
  }
  while(!learntClauses_.empty())
    learntClauses_.pop_back();
  for (int i = 0; i < learntClauses_.size(); i++) {
    learntClauses_.push_back(queue.top());
    queue.pop();
  }
  while(!queue.empty()) {
    Clause * c = queue.top();
    queue.pop();
    if (c->Locked()) {
      learntClauses_.push_back(c);
    } else {
      c->Remove(*this);
      delete c;
    }
  }
}
void Solver::RemoveFromWatchList(Lit &lit, Clause *clause) {
  int index = this->LitIndex(lit);
  watches_[index].erase(
      std::remove(watches_[index].begin(), watches_[index].end(), clause),
      watches_[index].end());
  for(auto c : watches_[index]){
    if (c == clause)
      throw "ERROR";
  }
}
LBool Solver::CheckConstraints() {
  for (auto c : constraints_) {
    if (!c->Value(*this)) {
      throw "should not happen";
    }
  }
  return LBool::kTrue;
}
void Solver::CheckWatchers() {
  for(auto c: constraints_) {
    c->CheckWatchers(this);
  }
  for(auto c: learntClauses_) {
    c->CheckWatchers(this);
  }


}
}

