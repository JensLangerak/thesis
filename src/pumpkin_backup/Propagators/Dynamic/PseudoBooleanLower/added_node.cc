//
// Created by jens on 29-04-21.
//

#include "added_node.h"
#include "../../../Basic Data Structures/problem_specification.h"
#include "../../../Engine/solver_state.h"

namespace Pumpkin {

AddedNode::AddedNode(WeightedLiteral literal1, WeightedLiteral literal2,
                     SolverState &state) {
  if (literal1.weight < literal2.weight) {
    inputs_.push_back(literal1);
    inputs_.push_back(literal2);
  } else {
    inputs_.push_back(literal2);
    inputs_.push_back(literal1);
  }

  BooleanLiteral o = BooleanLiteral();
  WeightedLiteral ow = WeightedLiteral(o, inputs_[0].weight);
  outputs_.push_back(ow);
  if (inputs_[0].weight != inputs_[1].weight) {
    //      o = BooleanLiteral(state.CreateNewVariable(), true);
    o = BooleanLiteral();
    ow = WeightedLiteral(o, inputs_[1].weight);
    outputs_.push_back(ow);
  }
  //   o = BooleanLiteral(state.CreateNewVariable(), true);
  o = BooleanLiteral();
  ow = WeightedLiteral(o, inputs_[0].weight + inputs_[1].weight);
  outputs_.push_back(ow);
  added_ = false;
}
void AddedNode::AddEncoding(SolverState& state) {
  assert(inputs_.size() == 2);
  std::vector<BooleanLiteral> sum_clause;
  for (WeightedLiteral w : inputs_) {
    sum_clause.push_back(~(w.literal));
    for (WeightedLiteral &o : outputs_) {
      if (o.literal.code_ == 0)
        o.literal = BooleanLiteral(state.CreateNewVariable(), true);
      if (w.weight == o.weight) {
        std::vector<BooleanLiteral> clause;
        clause.push_back(~(w.literal));
        clause.push_back(o.literal);
        auto added_clause = state.propagator_clausal_.clause_database_.AddPermanentClause(clause, state);
        clauses_.push_back(added_clause);
        if (state.assignments_.IsAssignedTrue(w.literal))
          state.EnqueuePropagatedLiteral(w.literal, &state.propagator_clausal_, reinterpret_cast<uint64_t>(added_clause));
      }
    }
  }

  assert(inputs_[0].weight + inputs_[1].weight == outputs_.back().weight);
  sum_clause.push_back(outputs_.back().literal);
  auto added_sum_clause = state.propagator_clausal_.clause_database_.AddPermanentClause(sum_clause, state);
  clauses_.push_back(added_sum_clause);
  if (state.assignments_.IsAssignedTrue(inputs_[0].literal) && state.assignments_.IsAssignedTrue(inputs_[1].literal))
    state.EnqueuePropagatedLiteral(outputs_.back().literal, &state.propagator_clausal_, reinterpret_cast<uint64_t>(added_sum_clause));
  added_ = true;
}
} // namespace Pumpkin