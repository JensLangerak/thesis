//
// Created by jens on 29-04-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANLOWER_ADDED_NODE_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANLOWER_ADDED_NODE_H_

#include "../../../Utilities//problem_specification.h"
#include "../../../Propagators/Clausal/two_watched_clause.h"
namespace Pumpkin {
class AddedNode {
public:
  AddedNode(WeightedLiteral literal1, WeightedLiteral literal2,
            SolverState& state);
  std::vector<WeightedLiteral> inputs_;
  std::vector<WeightedLiteral> outputs_;
  std::vector<TwoWatchedClause *> clauses_;
  int potential_use_count_ = 0;
  int use_count_ = 0;
  bool added_ = false;

  void AddEncoding(SolverState &state);
};
}

#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANLOWER_ADDED_NODE_H_
