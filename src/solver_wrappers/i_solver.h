//
// Created by jens on 18-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_SAT_I_SOLVER_H_
#define SIMPLESATSOLVER_SRC_SAT_I_SOLVER_H_

#include "../sat/sat_problem.h"
//TODO decide if this belongs here or with the wrappers
namespace simple_sat_solver::solver_wrappers {
class ISolver {
public:
  /// Return true when p is satisfiable, if unsatisfiable it returns false.
  /// After returning true, the solution can be obtained with GetSolution.
  /// \param p
  /// \return true when p is satisfiable.
  virtual bool Solve(const sat::SatProblem &p) = 0;
  virtual bool Optimize(const sat::SatProblem &p) = 0;

  /// Return the solution found be solve. Should only be used after Solver
  /// returns true.
  /// \return the solution found by solve.
  virtual std::vector<bool> GetSolution() const = 0;

  virtual ~ISolver();

protected:
  ISolver(){};
};
} // namespace simple_sat_solver::sat
#endif // SIMPLESATSOLVER_SRC_SAT_I_SOLVER_H_
