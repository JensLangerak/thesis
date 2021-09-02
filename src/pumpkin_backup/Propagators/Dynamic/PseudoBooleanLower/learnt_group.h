//
// Created by jens on 27-04-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANLOWER_LEARNT_GROUP_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANLOWER_LEARNT_GROUP_H_

#include "../../../Basic Data Structures/boolean_literal.h"
#include "../../../Basic Data Structures/problem_specification.h"
namespace Pumpkin {

class LearntGroup {
public:
  std::vector<BooleanLiteral> inputs_;
  std::vector<BooleanLiteral> outputs_;
  bool MatchedInput(std::vector<BooleanLiteral> causes, SolverState &state);
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANLOWER_LEARNT_GROUP_H_
