//
// Created by jens on 03-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_SOLVER_TYPES_H_
#define SIMPLESATSOLVER_SRC_SOLVER_TYPES_H_

#include <queue>
#include <vector>

#include "../sat/types.h"
namespace simple_sat_solver::solver {

template <typename T> using Vec = std::vector<T>;
template <typename T> using Queue = std::queue<T>;

using simple_sat_solver::sat::Var;
using simple_sat_solver::sat::Lit;

enum class LBool { kUnknown, kTrue, kFalse };

inline LBool operator~(LBool x) {
  switch (x) {
  case LBool::kUnknown:
    return LBool::kUnknown;
  case LBool::kTrue:
    return LBool::kFalse;
  case LBool::kFalse:
    return LBool::kTrue;
  }
  return LBool::kUnknown;
}

} // namespace simple_sat_solver::solver
#endif // SIMPLESATSOLVER_SRC_SOLVER_TYPES_H_
