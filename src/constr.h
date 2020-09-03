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
  virtual void Remove(Solver* s);
  virtual bool Propagate(Solver* s, Lit p);
  virtual bool Simplify(Solver* s);
  virtual void Undo(Solver* s, Lit p);
  virtual void CalcReason(Solver* s, Lit p, Vec<Lit> out_reason);
};
}

#endif //SIMPLESATSOLVER_SRC_CONSTR_H_
