//
// Created by jens on 25-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_SOLVER_WRAPPERS_CRYPTO_MINI_SAT_H_
#define SIMPLESATSOLVER_SRC_SOLVER_WRAPPERS_CRYPTO_MINI_SAT_H_

#include <cryptominisat5/cryptominisat.h>

#include "i_solver.h"

namespace simple_sat_solver::solver_wrappers {

class CryptoMiniSat : public ISolver {
  /// Class that uses the simple_sat_solver::solver to solve a sat problem
public:
  inline CryptoMiniSat() : solved_(false){};
  bool Solve(const sat::SatProblem &p) override;
  std::vector<bool> GetSolution() const override;
  ~CryptoMiniSat() {}

private:
  std::vector<bool> solution_;
  bool solved_;
};
} // namespace simple_sat_solver::solver_wrappers

#endif // SIMPLESATSOLVER_SRC_SOLVER_WRAPPERS_CRYPTO_MINI_SAT_H_
