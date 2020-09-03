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
  Var NewVar();
  bool AddClause(Vec<Lit> literals);
  bool SimplifyDb();
  bool Solve(Vec<Lit> assumptions);
  Vec<bool> Model();

 private:
  inline int NVars() { return assigns_.size(); }
  inline int NAssigns() { return trail_.size(); }
  inline int NConstraints() { return constrs_.size(); }
  inline int NLearnts() { return learnts_.size(); }
  inline LBool Value(Var x) { return assigns_[x]; }
  inline LBool value(Lit p) { return Sign(p) ? ~assigns_[GetVar(p)] : assigns_[GetVar(p)]; }
  inline int DecisionLevel() { return trail_lim_.size(); }
  // Internal state
  // Constraint database
  Vec<Constr> constrs_;
  Vec<Clause> learnts_;
  double cla_inc_;
  double cla_decay_;

  // Variable order
  Vec<double> activity_;
  double var_inc_;
  double var_decay_;
  VarOrder order_;

  // Propagation
  Vec<Vec<Constr>> watches_;
  Vec<Vec<Constr>> undos_;
  Queue<Lit> prop_q_;

  // Assignments
  Vec<LBool> assigns_;
  Vec<Lit> trail_;
  Vec<int> trail_lim_;
  Vec<Constr> reason_;
  Vec<int> level_;
  int root_level_;

};
}

#endif //SIMPLESATSOLVER_SRC_SOLVER_H_
