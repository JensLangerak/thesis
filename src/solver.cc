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
  reason_.push_back(nullptr);
  level_.push_back(-1);
  watches_.push_back(Vec<Constr*>());
  watches_.push_back(Vec<Constr*>());
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

bool Solver::CheckWatches() {
  for (int i =0; i < watches_.size(); i++) {
    Lit l;
    l.complement = i & 1;
    l.x = i >> 1;
    for (int j = 0; j < watches_[i].size(); j++) {
      Clause * clause = (Clause *) watches_[i][j];
      if (!(clause->lits_[clause->watchA] == l || clause->lits_[clause->watchB] == l)) {
       throw "error";
      }
      if (GetLitValue(l) == LBool::kFalse)
        throw "error";
      for (int l = j + 1; l < watches_[i].size(); l++) {
        if (watches_[i][j] == watches_[i][l])
          throw "error";
      }
    }
  }
  for (int i =0; i < constraints_.size(); i++) {
    Clause *clause = (Clause *) constraints_[i];
    int indexA = LitIndex(clause->lits_[clause->watchA]);
    int indexB = LitIndex(clause->lits_[clause->watchB]);
    bool fail = true;
    for (auto constr : watches_[indexA]) {
      if (constr == constraints_[i]){
        fail = false;
        break;
      }
    }
    if (fail)
      throw "error";
    fail = true;
    for (auto constr : watches_[indexB]) {
      if (constr == constraints_[i]){
        fail = false;
        break;
      }
    }
    if (fail)
      throw "error";

    if (indexA == indexB) {
      if (GetLitValue(clause->lits_[clause->watchA]) != LBool::kTrue) {
        for (int i = 0; i < clause->lits_.size(); i++) {

          if (GetLitValue(clause->lits_[i]) == LBool::kUnknown) {
            throw "error";
          }
        }
        if (GetLitValue(clause->lits_[clause->watchA]) == LBool::kFalse)
          throw "error";
      }
    } else {
      if (GetLitValue(clause->lits_[clause->watchA]) == LBool::kFalse)
        throw "error";

      if (GetLitValue(clause->lits_[clause->watchB]) == LBool::kFalse)
        throw "error";
    }

  }
  return true;
}

bool Solver::Solve() {
  //for (auto c: constraints_) {
  //  if (!c->Simplify(this))
  //    return false;
  //}
  bool stop = false;
  while (!stop) {
    if (!Propagate()) {
      while(!propagationQueue_.empty())
        propagationQueue_.pop();
      if (decisionLevels_.empty())
        return false;
      Vec<Lit> c = Analyze(conflictReason_);
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
      // TODO backtrack
      // Find if/how conflict is unit append to propagate

      if (!Backtrack(backtrackLevel))
        return false;
      CheckWatches();
      Clause *clause = new Clause(c, true, *this, unit);

      constraints_.push_back(clause);
      SetLitTrue(unit, clause);
    } else {
      CheckWatches();
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
bool Solver::SetLitTrue(Lit lit, Constr * constr) {
  LBool value = lit.complement ? LBool::kFalse : LBool::kTrue;
  Var x = lit.x;
  if (varAssignments_[x] == LBool::kUnknown) {
    //std::cout << "Set " << lit.x << " " << (lit.complement ? "F" : "T") << std::endl;
    varAssignments_[x] = value;
    propagationQueue_.push(~lit);
    learnt_.push(lit);
    level_[x] = (decisionLevels_.empty() ? -1 : decisionLevels_.top());
    reason_[x] = constr;
  } else if (varAssignments_[x] != value) {
    return false;
  }
  return true;
}
bool Solver::Propagate() {
  while(!propagationQueue_.empty()) {
    Lit lit = propagationQueue_.front();
    propagationQueue_.pop();
    //std::cout << "Propagat: " << lit.x << " " << (lit.complement ? "F" : "T") << std::endl;
    int index = LitIndex(lit);
    Vec<Constr *> watchList = watches_[index];
    watches_[index].clear();
    for (int i = 0; i < watchList.size(); i++) {
      auto c = watchList[i];
      if (GetLitValue(lit) == LBool::kUnknown)
        throw "error";
      if (!c->Propagate(this, lit)){
        this->conflictReason_ = c;
        for (; i < watchList.size(); i++)
          watches_[index].push_back(watchList[i]);
        return false;
      }
    }
  }
  return true;
}
LBool Solver::GetLitValue(Lit l) {
  LBool var = varAssignments_[l.x];
  return l.complement ? ~var : var;
}

bool Solver::UndoOne() {
  if (learnt_.empty())
    return false;
  Lit l = learnt_.top();
  learnt_.pop();
  varAssignments_[l.x] = LBool::kUnknown;
  //std::cout << "Undo: " << l.x << std::endl;
  level_[l.x] = -1;
  Lit complL = ~l;
  for (Constr * c : watches_[LitIndex(l)]) {
    Clause * cl = (Clause * ) c;
    ((Clause *) c)->UpdateWatches(this);
  }
  for (Constr * c : watches_[LitIndex(complL)]) {
    Clause * cl = (Clause * ) c;
    ((Clause *) c)->UpdateWatches(this);
  }
  reason_[l.x] = nullptr;
  return true;
}

void Solver::UndoDecisions(int level) {
  while (learnt_.size() > level) {
    UndoOne();
  }
}
bool Solver::Backtrack(int level) {
  while ((decisionLevels_.empty() ? -1 : decisionLevels_.top()) > level) {
    //std::cout << "Pop: " << decisionLevels_.top() << std::endl;
    UndoDecisions(decisionLevels_.top());
    decisionLevels_.pop();
  }
  return true;
}
bool Solver::Backtrack() {
    while (!decisionLevels_.empty()) {
      UndoDecisions(decisionLevels_.top() + 1);
      Lit lastDecision = learnt_.top();
      UndoDecisions(decisionLevels_.top());
      decisionLevels_.pop();
      //std::cout << "Pop: " << lastDecision.x << std::endl;
      if (!lastDecision.complement) {
        lastDecision.complement = true;
        Assume(lastDecision);
        return true;
      }
    }

  return false;
}
void Solver::Assume(Lit lit) {
 //std::cout <<learnt_.size() <<  " Assume: " << lit.x << (lit.complement ? "F" : "T") << std::endl;
  decisionLevels_.push(learnt_.size());
  SetLitTrue(lit, nullptr);
}
bool Solver::AllAssigned() {
  for (LBool b : varAssignments_) {
    if (b == LBool::kUnknown)
      return false;
  }
  return true;
}
Vec<Lit> Solver::Analyze(Constr *constr) {
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
      if (level_[l.x] == decisionLevels_.top()) {
        ++level_count;
      } else {
        learnt.push_back(l);
      }
    }

    // go back in time, doing so will reveal an uip
    Lit p;
    Constr* r;
    do {
      p = learnt_.top();
      r = reason_[p.x];
      UndoOne();
    } while(!seen[p.x]);
    if (level_count <= 1) { // UIP found
      learnt.push_back(~p);
      break;
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

}