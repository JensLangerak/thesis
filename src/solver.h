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

  void PrintProblem();
  void PrintAssignments();
  void PrintFilledProblem();

  bool SetLitTrue(Lit lit, Constr *constr);
  LBool GetLitValue(Lit l);
  void AddWatch(Lit &lit, Clause *p_clause);

  double constrIncActivity;
  double constrDecayFactor;
  void RescaleClauseActivity();
  void RemoveFromWatchList(Lit &lit, Clause *p_clause);
  Queue<Lit> propagationQueue_;
  Vec<Vec<Constr*>> watches_;
  static int LitIndex(Lit &lit);
 private:
  bool Propagate();

  //TODO perhaps add undo list
  VarOrder varOrder;
  Vec<bool> model_; //TODO not sure if needed
  Vec<Constr*> constraints_;
  Vec<Clause*> learntClauses_;
  Vec<LBool> varAssignments_;
  Vec<int> level_;
  Vec<Constr*> reason_;
  Constr* conflictReason_; //TODO don't like this

  std::stack<Lit> learnt_;

  std::stack<int> decisionLevels_;

  void Assume(Lit lit);
  bool AllAssigned();
  void UndoDecisions(int level);
  Vec<Lit> Analyze(Constr *p_constr);
  bool Backtrack(int level);
  bool UndoOne();
  int GetMostRecentLitIndex(Vec<Lit> lits);
  bool HandleConflict();
  bool AddAssumption();
  LBool Solve(int maxLearnt);
  void ReduceDB(int learnt);
  LBool CheckConstraints();
  void CheckWatchers();
};
}

#endif //SIMPLESATSOLVER_SRC_SOLVER_H_
