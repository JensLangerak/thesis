//
// Created by jens on 07-06-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PAIRS_PAIRS_DATABASE_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PAIRS_PAIRS_DATABASE_H_

#include <unordered_map>
#include "added_node.h"
namespace Pumpkin {
class PairsDatabase {
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
  void AddScheduled(SolverState & state);
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PAIRS_PAIRS_DATABASE_H_
