//
// Created by jens on 29-04-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANLOWER_NODE_DATABASE_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANLOWER_NODE_DATABASE_H_

#include <unordered_map>
#include <unordered_set>

#include "../../../Basic Data Structures/boolean_literal.h"
#include "../../../Basic Data Structures/problem_specification.h"
#include "added_node.h"


namespace Pumpkin {
class NodeDatabase {
private:
  struct hash
  {
    std::size_t operator () (BooleanLiteral const &v) const
    {
      return std::hash<int>()(v.code_);
    }
  };
public:
//private:
  std::unordered_map<BooleanLiteral, std::unordered_map<BooleanLiteral, AddedNode*, hash>, hash> pairs_;
  std::vector<AddedNode *> scheduled_;

  void CreateNode(WeightedLiteral &literal1, WeightedLiteral &literal2,
                  SolverState &state);
  void AddToPairs(WeightedLiteral &key, WeightedLiteral &subkey,
                  AddedNode *node);


};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANLOWER_NODE_DATABASE_H_
