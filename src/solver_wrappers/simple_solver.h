//
// Created by jens on 18-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_SAT_SOLVERS_SIMPLE_SOLVER_H_
#define SIMPLESATSOLVER_SRC_SAT_SOLVERS_SIMPLE_SOLVER_H_
#include "i_solver.h"
namespace simple_sat_solver::solver_wrappers {
class SimpleSolver : public ISolver {
  /// Class that uses the simple_sat_solver::solver to solve a sat problem
public:
  inline SimpleSolver() : solved_(false){};
  bool Solve(const sat::SatProblem &p) override;
  std::vector<bool> GetSolution() const override;
  ~SimpleSolver() {}

private:
  std::vector<bool> solution_;
  bool solved_;
};
} // namespace simple_sat_solver::solver_wrappers

#endif // SIMPLESATSOLVER_SRC_SAT_SOLVERS_SIMPLE_SOLVER_H_
