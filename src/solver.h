//
// Created by jens on 03-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_SOLVER_H_
#define SIMPLESATSOLVER_SRC_SOLVER_H_

#include <stack>
#include "types.h"
#include "var_order.h"
#include "constr.h"
#include "clause.h"
namespace simple_sat_solver {
//class Clause;
//class Constr;
class Solver {
 public:
  Solver();
  ~Solver();
  Var NewVar();
  bool AddClause(const Vec<Lit> &literals);
  bool SimplifyDb();
  bool Solve(const Vec<Lit> &assumptions);
  bool Solve();
  inline Vec<bool> GetModel() { return model_; };

  const void PrintProblem();
  const void PrintAssignments();
  const void PrintFilledProblem();

  bool SetLitTrue(Lit lit, Constr *constr);
  LBool GetLitValue(Lit l);
  void AddWatch(Lit &lit, Clause *p_clause);
 private:
  bool Propagate();

  Vec<bool> model_; //TODO not sure if needed
  Vec<Constr*> constraints_;
  Vec<LBool> varAssignments_;
  Vec<int> level_;
  Vec<Constr*> reason_;
  Vec<Vec<Constr*>> watches_;
  Constr* conflictReason_;
  Queue<Lit> propagationQueue_;

  std::stack<Lit> learnt_;

  std::stack<int> decisionLevels_;

  bool Backtrack();
  void Assume(Lit lit);
  bool AllAssigned();
  void UndoDecisions(int level);
  Vec<Lit> Analyze(Constr *p_constr);
  bool Backtrack(int level);
  bool UndoOne();
  static int LitIndex(Lit &lit);
  int GetMostRecentLitIndex(Vec<Lit> lits);
  bool HandleConflict();
  bool AddAssumption();
};
}

#endif //SIMPLESATSOLVER_SRC_SOLVER_H_
