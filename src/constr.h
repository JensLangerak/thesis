//
// Created by jens on 03-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_CONSTR_H_
#define SIMPLESATSOLVER_SRC_CONSTR_H_

//#include "solver.h"
#include "types.h"

namespace simple_sat_solver {
class Solver;
class Constr {
 public:
  virtual ~Constr();
  virtual void Remove(Solver* s);
  virtual bool Propagate(Solver* s, Lit p);
  virtual bool Simplify(Solver* s);
  virtual void Undo(Solver* s, Lit p);
  virtual Vec<Lit> CalcReason(Lit p);
  virtual Vec<Lit> CalcReason();
  virtual const void PrintConstraint();
  virtual const void PrintFilledConstraint(const Vec<LBool> &vars);
};
}

#endif //SIMPLESATSOLVER_SRC_CONSTR_H_
