//
// Created by jens on 21-03-21.
//

#include "generalized_totliser_sum_root.h"

#include "../../../Engine/solver_state.h"
#include "generalized_totaliser_sum_helpers.h"
#include "generalized_totaliser_sum_nodes.h"
#include "../Sum/watched_pb_sum_constraint.h"
#include <algorithm>
#include <iostream>
#include <stack>
#include <utility>

namespace Pumpkin {

void GeneralizedTotliserSumRoot::Encode(SolverState &state) {
//  std::cout << "Encode root " << std::endl;
  if (max_ == 0) {
    for (auto wl : inputs_) {
      BooleanLiteral l = ~(wl.literal);
      state.AddUnitClause(l);
    }
    encoding_added_  = true;
    return;

  }
  encoding_added_  = true;
  std::vector<WeightedLiteral> outputs = GeneralizedTotaliserSumHelpers::CreateSumLiterals(inputs_, max_, state);
  this->root = GeneralizedTotaliserSumHelpers::AddPbSumConstraint(inputs_, outputs, max_, add_delay, encoding_strategy_, state,
      root_node_);
  this->root->encoder_->Encode(state);
}

void GeneralizedTotliserSumRoot::Encode(SolverState &state,
                                   std::vector<BooleanLiteral> lits) {
  assert(false);
}
GeneralizedTotliserSumRoot::GeneralizedTotliserSumRoot(
    std::vector<BooleanLiteral> variables, std::vector<uint32_t> weights,
    int max) : max_(max), root_node_(nullptr){
  assert(variables.size() == weights.size());
  inputs_.reserve(variables.size());
  for (int i = 0; i < variables.size(); ++i) {
    inputs_.push_back(WeightedLiteral(variables[i], weights[i]));
  }
}
bool GeneralizedTotliserSumRoot::UpdateMax(int max, SolverState &state) {
  max_ = max;
  if (root_node_ != nullptr) {
    {
//      root->max_ = max + 1;
//      BooleanLiteral l = BooleanLiteral(state.CreateNewVariable(), true);
//      root->outputs_.push_back(WeightedLiteral(l, max + 1));
//      root->lit_to_output_index_[l.ToPositiveInteger()] = root->outputs_.size()- 1;
//      root->weight_output_index_map_[max + 1] = root->outputs_.size()-1;
//      root_node_->outputs_.push_back(WeightedLiteral(l, max+1));
      assert(root->weight_output_index_map_.count(max + 1) > 0);
    }
    return root_node_->UpdateMax(max, state);
  }
  return true;
}
}