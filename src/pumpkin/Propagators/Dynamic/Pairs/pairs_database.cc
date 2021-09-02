//
// Created by jens on 07-06-21.
//

#include "pairs_database.h"

namespace Pumpkin {
void PairsDatabase::CreateNode(WeightedLiteral &literal1,
                              WeightedLiteral &literal2, SolverState &state) {

  if (pairs_.count(literal1.literal) >= 0 && pairs_[literal1.literal].count(literal2.literal) > 0) // Sum value is true due to decision
    return;
  AddedNode * node = new AddedNode(literal1, literal2, state);
  scheduled_.push_back(node);
  AddToPairs(literal1, literal2, node);
  AddToPairs(literal2, literal1, node);
}
void PairsDatabase::AddToPairs(WeightedLiteral &key, WeightedLiteral &subkey,
                              AddedNode *node) {
  if (pairs_.count(key.literal) == 0) {
    pairs_[key.literal] = std::unordered_map<BooleanLiteral, AddedNode*, hash>();
  }
  if(pairs_[key.literal].count(subkey.literal) > 0) {
    assert(pairs_[key.literal].count(subkey.literal) == 0);
  }
  pairs_[key.literal][subkey.literal] = node;
}
void PairsDatabase::AddScheduled(SolverState & state) {
  for (auto s : scheduled_) {
    s->AddEncoding(state);
  }
  scheduled_.clear();
}
}