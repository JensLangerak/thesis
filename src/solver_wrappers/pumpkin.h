//
// Created by jens on 15-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_SOLVER_WRAPPERS_PUMPKIN_H_
#define SIMPLESATSOLVER_SRC_SOLVER_WRAPPERS_PUMPKIN_H_

#include "i_solver.h"
namespace simple_sat_solver::solver_wrappers {
class Pumpkin : public ISolver {
public:
  inline Pumpkin() : solved_(false){};
  bool Solve(const sat::SatProblem &p) override;
  std::vector<bool> GetSolution() const override;
  ~Pumpkin() {}

private:
  std::vector<bool> solution_;
  bool solved_;

};
}

#endif // SIMPLESATSOLVER_SRC_SOLVER_WRAPPERS_PUMPKIN_H_
