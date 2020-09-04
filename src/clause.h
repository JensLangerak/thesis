//
// Created by jens on 03-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_CLAUSE_H_
#define SIMPLESATSOLVER_SRC_CLAUSE_H_

#include "constr.h"
namespace simple_sat_solver {
class Solver;
class Clause : public Constr {
 public:
//static bool NewClause(Solver &S, Vec<Lit> ps, bool learnt, Clause &out_clause);
  Clause(Vec<Lit> ps, bool learnt);
  ~Clause() override;

  bool Locked(Solver* s);
  void Remove(Solver* s) override;

  bool Simplify(Solver* S) override;

  bool Propagate(Solver* S, Lit p) override;

  void Undo(Solver* s, Lit p) override;

  void CalcReason(Solver* S, Lit p, Vec<Lit> out_reason) override;
  const void PrintConstraint() override;
  const void PrintFilledConstraint(const Vec<LBool> &vars) override;


 private:
  bool learnt_;
  Vec<Lit> lits_;
};
}

#endif //SIMPLESATSOLVER_SRC_CLAUSE_H_
