//
// Created by jens on 03-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_SOLVER_H_
#define SIMPLESATSOLVER_SRC_SOLVER_H_

#include "types.h"
namespace simple_sat_solver {
class Solver {
 public:
  Var NewVar();
  bool AddClause(Vec<Lit> literals);
  bool SimplifyDb();
  bool Solve(Vec<Lit> assumptions);
  Vec<bool> Model();
};
}

#endif //SIMPLESATSOLVER_SRC_SOLVER_H_
