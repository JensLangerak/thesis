//
// Created by jens on 29-04-21.
//

#include "node_database.h"
namespace Pumpkin {



void NodeDatabase::CreateNode(WeightedLiteral &literal1,
                              WeightedLiteral &literal2, SolverState &state) {

  if (pairs_.count(literal1.literal) >= 0 && pairs_[literal1.literal].count(literal2.literal) > 0) // Sum value is true due to decision
    return;
  AddedNode * node = new AddedNode(literal1, literal2, state);
  scheduled_.push_back(node);
  AddToPairs(literal1, literal2, node);
  AddToPairs(literal2, literal1, node);
}
void NodeDatabase::AddToPairs(WeightedLiteral &key, WeightedLiteral &subkey,
                              AddedNode *node) {
  if (pairs_.count(key.literal) == 0) {
    pairs_[key.literal] = std::unordered_map<BooleanLiteral, AddedNode*, hash>();
  }
  if(pairs_[key.literal].count(subkey.literal) > 0) {
    auto tes = pairs_[key.literal][subkey.literal];
    assert(pairs_[key.literal].count(subkey.literal) == 0);
  }
  pairs_[key.literal][subkey.literal] = node;
}
}
