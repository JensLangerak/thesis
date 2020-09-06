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
      if (decisionLevels_.empty())
        return false;
      Vec<Lit> c = Analyze(conflictReason_);
      Clause *clause = new Clause(c, true);
      int backtrackLevel = -1;
      Lit unit;
      for(Lit l : c) {
        int level = level_[l.x];
        if (level != decisionLevels_.top()) {
          backtrackLevel = std::max(backtrackLevel, level);
        } else {
          unit = l;
        }
      }
      // TODO backtrack
      // Find if/how conflict is unit append to propagate

      if (!Backtrack(backtrackLevel))
        return false;
      constraints_.push_back(clause);
      SetLitTrue(unit, clause);
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
bool Solver::SetLitTrue(Lit lit, Constr * constr) {
  LBool value = lit.complement ? LBool::kFalse : LBool::kTrue;
  Var x = lit.x;
  if (varAssignments_[x] == LBool::kUnknown) {
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
    for (auto c : constraints_) {
      if (!c->Propagate(this, lit)){
        this->conflictReason_ = c;
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

void Solver::UndoDecisions(int level) {
  while (learnt_.size() > level) {
    Lit l = learnt_.top();
    learnt_.pop();
    varAssignments_[l.x] = LBool::kUnknown;
    level_[l.x] = -1;
    reason_[l.x] = nullptr;
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
  std::queue<Lit> conflictReason;
  for (Lit l : conflictReason_->CalcReason()) {
    conflictReason.push(l);
  }
  while(!conflictReason.empty()) {
    Lit l = conflictReason.front();
    conflictReason.pop();
    if (seen[l.x])
      continue;
    seen[l.x] = true;
    // todo check if (assert l == false)
    if (level_[l.x] == decisionLevels_.top()) {
      if (reason_[l.x] == nullptr) {
        learnt.push_back(l);
      } else {
        for (Lit l : reason_[l.x]->CalcReason(l)) {
          conflictReason.push(l);
        }
      }
    } else { // TODO exclude top
      learnt.push_back(l);
    }
  }
  return learnt;
}

}