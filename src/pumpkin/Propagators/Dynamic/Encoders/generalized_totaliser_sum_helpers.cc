//
// Created by jens on 22-03-21.
//

#include "generalized_totaliser_sum_helpers.h"
#include "../../../Engine/solver_state.h"
#include "generalized_totaliser_sum_nodes.h"
#include <algorithm>
#include <stack>
#include <utility>
namespace Pumpkin {

std::vector<WeightedLiteral> GeneralizedTotaliserSumHelpers::CreateSumLiterals(
    const std::vector<WeightedLiteral>& inputs, int max, SolverState &state) {

  //For testing remove later
  {
//    BooleanLiteral l = BooleanLiteral(state.CreateNewVariable(), true);
//    WeightedLiteral wl = WeightedLiteral(l, max + 1);
//    std::vector<WeightedLiteral> outputs;
//    outputs.push_back(wl);
//      BooleanLiteral nl = ~l;
//      state.AddUnitClause(nl);
//      return outputs;
  }

  //end testing


  std::set<int, std::less<> > weights;
  bool add_max = false;

  for (WeightedLiteral wl : inputs) {
    std::stack<int> new_sums;
    if (wl.weight > max) {
      add_max = true;
    } else {
      new_sums.push(wl.weight);
    }
    for (int w : weights) {
      int new_w = w + wl.weight;
      if (new_w > max) {
        add_max = true;
      } else {
        new_sums.push(w + wl.weight);
      }
    }
    while (!new_sums.empty()) {
      int w = new_sums.top();
      new_sums.pop();
      weights.insert(w);
    }
  }
  if (add_max) {
    weights.insert(max + 1);
  }

  std::vector<WeightedLiteral> outputs;
  outputs.reserve(weights.size());
  for(int w : weights) {
    BooleanLiteral l = BooleanLiteral(state.CreateNewVariable(), true);
    WeightedLiteral wl = WeightedLiteral(l, w);
    outputs.push_back(wl);
    if (w > max) {
      BooleanLiteral nl = ~l;
      state.AddUnitClause(nl);
    }
  }
  if (outputs.size() > max + 1) {
    assert(outputs.size() <= max + 1);
  }
  return outputs;
}
WatchedPbSumConstraint* GeneralizedTotaliserSumHelpers::AddPbSumConstraint(
    std::vector<WeightedLiteral> inputs, std::vector<WeightedLiteral> outputs,
    int max, double add_delay, bool add_incremental, bool add_dynamic,
    SolverState &state, GeneralizedTotaliserSumNodes *& encoder_result) {
  std::vector<BooleanLiteral> input_lits;
  std::vector<uint32_t> input_weighs;
  for (WeightedLiteral w : inputs) {
    input_lits.push_back(w.literal);
    input_weighs.push_back(w.weight);
  }

  std::vector<BooleanLiteral> output_lits;
  std::vector<uint32_t> output_weighs;
  for (WeightedLiteral w : outputs) {
    output_lits.push_back(w.literal);
    output_weighs.push_back(w.weight);
  }
  auto * encoder = new GeneralizedTotaliserSumNodes(inputs, outputs, max);
  encoder->add_delay = add_delay;
  encoder->add_incremental = add_incremental;
  encoder->add_dynamic_ = add_dynamic;
  encoder_result = encoder;

  return state.propagator_pb_sum_.sum_database_.AddPermanentConstraint(input_lits, input_weighs, output_lits, output_weighs, encoder, state);


}
}