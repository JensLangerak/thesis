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
  Clause(Vec<Lit> ps, bool learnt, Solver &s);
  Clause(Vec<Lit> lits, bool learnt, Solver &s, Lit unitLit, int mostRecentLearntIndex);
  ~Clause() override;

  bool Locked(Solver* s);
  void Remove(Solver* s) override;

  bool Simplify(Solver* S) override;

  bool Propagate(Solver* S, Lit p) override;

  void Undo(Solver* s, Lit p) override;

  Vec<Lit> CalcReason(Lit p) override;
  Vec<Lit> CalcReason() override;
  const void PrintConstraint() override;
  const void PrintFilledConstraint(const Vec<LBool> &vars) override;
  void UndoUnitWatch(Solver *s) override;


  private:
  int watchA_;
  int watchB_;
  int watchLast;
  Vec<Lit> lits_;
  bool learnt_;
};
}

#endif //SIMPLESATSOLVER_SRC_CLAUSE_H_
