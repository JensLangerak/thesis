//
// Created by jens on 03-09-20.
//

#include "solver.h"

#include <algorithm>
#include <iostream>

#include "var_order.h"

namespace simple_sat_solver::solver {
Solver::Solver()
    : constr_inc_activity_(1.0), constr_decay_factor_(1.0 / 0.95) {}

Var Solver::NewVar() {
  var_assignments_.push_back(LBool::kUnknown);
  reason_.push_back(nullptr);
  level_.push_back(-1);
  // watch list for true and false lit.
  watches_.push_back(Vec<Clause *>());
  watches_.push_back(Vec<Clause *>());

  var_order_.NewVar();
  return var_assignments_.size() - 1;
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
  for (auto c : learnt_clauses_)
    delete c;
  constraints_.clear();
}

void Solver::PrintProblem() {
  std::cout << "Nr vars: " << var_assignments_.size() << std::endl;
  for (auto c : constraints_)
    c->PrintConstraint();
}

void Solver::PrintAssignments() {
  for (int i = 0; i < var_assignments_.size(); i++) {
    LBool v = var_assignments_[i];
    std::cout << i << ": "
              << (v == LBool::kTrue    ? "T"
                  : v == LBool::kFalse ? "F"
                                       : "U")
              << std::endl;
  }
}

void Solver::PrintFilledProblem() {
  std::cout << "Nr vars: " << var_assignments_.size() << std::endl;
  for (auto c : constraints_)
    c->PrintFilledConstraint(var_assignments_);
}

bool Solver::Solve() {
  int max_learnt = constraints_.size() / 3;
  int max_conflicts = 100;
  LBool res = LBool::kUnknown;
  while (res == LBool::kUnknown) {
    res = Solve(max_learnt, max_conflicts);
    max_learnt *= 1.1;
    max_conflicts *= 1.5;
  }
  return res == LBool::kTrue;
}

LBool Solver::Solve(int max_learnt, int max_conflicts) {
  for (auto c : constraints_) {
    if (!c->Simplify(*this))
      return LBool::kFalse;
  }
  for (auto c : learnt_clauses_) {
    if (!c->Simplify(*this))
      return LBool::kFalse;
  }
  bool stop = false;
  while (!stop) {
    Clause *conflict;
    if (!Propagate(conflict)) { // conflict found
      max_conflicts--;
      while (!propagation_queue_.empty())
        propagation_queue_.pop();
      if (max_conflicts < 0) {
        Backtrack(0);
        return LBool::kUnknown;
      }
      if (!HandleConflict(conflict))
        return LBool::kFalse;
    } else {
      if (learnt_clauses_.size() > max_learnt) {
        ReduceDB(max_learnt);
        Backtrack(0);
        while (!propagation_queue_.empty())
          propagation_queue_.pop();
        return LBool::kUnknown;
      }
      stop = !AddAssumption();
      constr_inc_activity_ *= constr_decay_factor_;
    }
  }

  return AllAssigned() ? CheckConstraints() : LBool::kUnknown;
}

bool Solver::SetLitTrue(Lit lit, Clause *constr) {
  LBool value = lit.complement ? LBool::kFalse : LBool::kTrue;
  Var x = lit.x;
  if (var_assignments_[x] == LBool::kUnknown) {
    var_assignments_[x] = value;
    propagation_queue_.push(~lit);
    learnt_.push(lit);
    level_[x] = (decision_levels_.empty() ? 0 : decision_levels_.top());
    reason_[x] = constr;
    if (constr != nullptr)
      constr->Lock();
  } else if (var_assignments_[x] != value) {
    return false;
  }
  return true;
}

bool Solver::Propagate(Clause *&conflict) {
  while (!propagation_queue_.empty()) {
    Lit lit = propagation_queue_.front();
    propagation_queue_.pop();
    int index = LitIndex(lit);
    // clear original watch list, clauses should re-add themselves to the list.
    Vec<Clause *> watch_list = watches_[index];
    watches_[index].clear();

    for (int i = 0; i < watch_list.size(); i++) {
      auto c = watch_list[i];
      if (GetLitValue(lit) == LBool::kUnknown)
        throw "Should not propagate unknown literals";
      if (!c->Propagate(*this, lit)) {
        // conflict found
        conflict = c;
        // re-add the unhandled watches, excluding the current one.
        for (i = i + 1; i < watch_list.size(); i++)
          watches_[index].push_back(watch_list[i]);
        return false;
      }
    }
  }
  return true;
}

LBool Solver::GetLitValue(Lit l) const {
  LBool var = var_assignments_[l.x];
  return l.complement ? ~var : var;
}

bool Solver::UndoOne() {
  if (learnt_.empty())
    return false;

  Lit l = learnt_.top();
  learnt_.pop();
  var_assignments_[l.x] = LBool::kUnknown;
  var_order_.Undo(l.x);

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
  while (!decision_levels_.empty() && decision_levels_.top() > level) {
    UndoDecisions(decision_levels_.top());
    decision_levels_.pop();
  }
  return true;
}

void Solver::Assume(Lit lit) {
  decision_levels_.push(learnt_.size() + 1);
  SetLitTrue(lit, nullptr);
}

bool Solver::AllAssigned() const {
  for (LBool b : var_assignments_) {
    if (b == LBool::kUnknown)
      return false;
  }
  return true;
}

Vec<Lit> Solver::Analyze(const Clause *constr) {
  Vec<Lit> learnt;
  Vec<bool> seen(var_assignments_.size());
  Vec<Lit> conflict_reason = constr->CalcReason();
  int level_count = 0; // Keeps track of vars in the current decision level.
  do {
    // handle current conflict reasons
    for (Lit l : conflict_reason) {
      if (seen[l.x])
        continue;
      seen[l.x] = true;
      var_order_.Update(l.x);
      if (level_[l.x] == decision_levels_.top()) {
        ++level_count;
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
    if (level_count <= 1) { // UIP found
      learnt.push_back(~p);
      return learnt;
    }
    conflict_reason = r->CalcReason(p);
    --level_count;
  } while (level_count > 0); // UIP found

  return learnt;
}

void Solver::AddWatch(Lit &lit, Clause *clause) {
  watches_[LitIndex(lit)].push_back(clause);
}

int Solver::LitIndex(Lit &lit) { return lit.x * 2 + (lit.complement ? 1 : 0); }
Lit Solver::GetMostRecentLit(const Vec<Lit> &lits) {
  // values that are popped from learnt are temporarily stored in hist.
  std::stack<Lit> hist;
  Lit last;
  last.x = -1;
  while (!learnt_.empty()) {
    for (auto &lit : lits) {
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
  }
  return last;
}

bool Solver::HandleConflict(const Clause *conflict) {
  if (decision_levels_.empty()) // cannot backtrack, so un sat
    return false;

  Vec<Lit> c = Analyze(conflict);
  var_order_.UpdateAll();

  // Find the clause that will be unit and find the lowest backtrack_level
  int backtrack_level = -1;
  Lit unit;
  for (Lit l : c) {
    int level = level_[l.x];
    if (level != -1) {
      backtrack_level = std::max(backtrack_level, level);
    } else {
      unit = l;
    }
  }

  if (!Backtrack(backtrack_level)) // cannot backtrack, so it is unsatisfiable
    return false;

  Lit most_recent_lit = GetMostRecentLit(c); // needed for the watchers
  Clause *clause = new Clause(c, true, *this, unit, most_recent_lit);

  // Added clause is unit, so set lit
  learnt_clauses_.push_back(clause);
  SetLitTrue(unit, clause);
  return true;
}

bool Solver::AddAssumption() {
  Var v = var_order_.Select(var_assignments_);
  if (v < 0)
    return false;
  Assume(Lit(v, false));
  return true;
}

void Solver::RescaleClauseActivity() {
  for (Clause *c : learnt_clauses_)
    c->RescaleActivity();
  constr_inc_activity_ *= 1e-100;
}

void Solver::ReduceDB(int learnt) {
  auto comp = [](Clause *a, Clause *b) {
    return a->GetActivity() > b->GetActivity();
  };

  std::priority_queue<Clause *, Vec<Clause *>, decltype(comp)> queue(comp);
  for (Clause *c : learnt_clauses_) {
    queue.push(c);
  }
  while (!learnt_clauses_.empty())
    learnt_clauses_.pop_back();
  for (int i = 0; i < learnt_clauses_.size(); i++) {
    learnt_clauses_.push_back(queue.top());
    queue.pop();
  }
  while (!queue.empty()) {
    Clause *c = queue.top();
    queue.pop();
    if (c->Locked()) {
      learnt_clauses_.push_back(c);
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
  for (auto c : learnt_clauses_) {
    c->CheckWatchers(this);
  }
}
Vec<bool> Solver::GetModel() const {
  Vec<bool> res(var_assignments_.size());
  for (int i = 0; i < var_assignments_.size(); i++) {
    switch (var_assignments_[i]) {
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
  return res;
}
} // namespace simple_sat_solver::solver
