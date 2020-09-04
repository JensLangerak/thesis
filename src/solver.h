//
// Created by jens on 03-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_SOLVER_H_
#define SIMPLESATSOLVER_SRC_SOLVER_H_

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
  bool AddClause(Vec<Lit> literals);
  bool SimplifyDb();
  bool Solve(Vec<Lit> assumptions);
  inline Vec<bool> GetModel() { return model_; };

  void PrintProblem();
  void PrintAssinments();
  void PrintFilledProblem();
 private:
  Vec<bool> model_; //TODO not sure if needed
  Vec<Constr*> constraints_;
  Vec<LBool> varAssignments_;
};
}

#endif //SIMPLESATSOLVER_SRC_SOLVER_H_
