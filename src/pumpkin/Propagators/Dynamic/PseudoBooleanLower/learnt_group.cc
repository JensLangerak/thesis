//
// Created by jens on 27-04-21.
//

#include "learnt_group.h"
#include "../../../Engine/solver_state.h"

namespace Pumpkin {

bool LearntGroup::MatchedInput(std::vector<BooleanLiteral> causes,
                               SolverState &state) {
  for (BooleanLiteral l : inputs_) {
    if (state.assignments_.IsAssignedTrue(l)) {
      for (BooleanLiteral l2 : causes) {
        if (l2 == l)
          continue;
      }
      return false;
    }
  }
  return true;
}
}