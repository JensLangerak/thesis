//
// Created by jens on 25-05-21.
//

#include "sum_node.h"
#include "../../../Engine/solver_state.h"

namespace Pumpkin {

SumNode::SumNode(WeightedLiteral literal, WeightedLiteral literal_1,
                 SolverState &state) : current_sum_(0){
  WeightedLiteral small = literal;
  WeightedLiteral large = literal_1;
  if (small.weight > large.weight) {
    large = small;
    small = literal_1;
  }

  BooleanLiteral l = BooleanLiteral(state.CreateNewVariable(), true);
  state.AddBinaryClause(l, ~(small.literal));
  sum_literals.push_back(WeightedLiteral(l, small.weight));


  if (small.weight != large.weight) {
    l = BooleanLiteral(state.CreateNewVariable(), true);
    sum_literals.push_back(WeightedLiteral(l, large.weight));
  }
  state.AddBinaryClause(l, ~(large.literal));
  l = BooleanLiteral(state.CreateNewVariable(), true);
  std::vector<BooleanLiteral> clause {l, ~(large.literal), ~(small.literal)};
  state.AddClause(clause);
  sum_literals.push_back(WeightedLiteral(l, small.weight+ large.weight));

}
}