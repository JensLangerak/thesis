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
  static bool NewClause(Solver S, Vec<Lit> ps, bool learnt, Clause out_clause);

  bool Locked(Solver* s);
  void Remove(Solver* s);

  bool Simplify(Solver* S);

  bool Propagate(Solver* S, Lit p);

  void CalcReason(Solver* S, Lit p, Vec<Lit> out_reason);

 private:
  Clause();

  bool learnt_;
  float activity_;
  Vec<Lit> lits_;
};
}

#endif //SIMPLESATSOLVER_SRC_CLAUSE_H_
