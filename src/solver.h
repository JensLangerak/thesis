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

  bool SetLitTrue(Lit lit);
  const void PrintProblem();
  const void PrintAssinments();
  const void PrintFilledProblem();

  const LBool GetLitValue(Lit l);
 private:
  bool Propagate();

  Vec<bool> model_; //TODO not sure if needed
  Vec<Constr*> constraints_;
  Vec<LBool> varAssignments_;
  Queue<Lit> propagationQueue_;

  std::stack<Lit> assumptions_;
  bool Backtrack();
  void Assume(Lit lit);
  bool AllAssigned();
};
}

#endif //SIMPLESATSOLVER_SRC_SOLVER_H_
