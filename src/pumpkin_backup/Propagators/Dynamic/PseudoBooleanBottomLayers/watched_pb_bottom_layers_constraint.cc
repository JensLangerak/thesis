//
// Created by jens on 18-05-21.
//

#include "watched_pb_bottom_layers_constraint.h"

#include "../../../Basic Data Structures/boolean_literal.h"
#include "../../../Engine/solver_state.h"
#include "../Encoders/generalized_totaliser.h"
#include "explanation_pb_bottom_layers.h"

namespace Pumpkin {
WatchedPbBottomLayersConstraint::WatchedPbBottomLayersConstraint(
    std::vector<BooleanLiteral> &inputs, std::vector<uint32_t> &input_weights,
   int max)
    : true_count_(0) , debug_index(next_debug_index), max_(max){
  next_debug_index++;
  assert(inputs.size() == input_weights.size());
  inputs_.reserve(inputs.size());
  for (int i = 0; i < inputs.size(); ++i) {
    inputs_.push_back(WeightedLiteral(inputs[i], input_weights[i]));
    lit_to_input_index_[inputs[i].ToPositiveInteger()] = i;
    if (input_weights[i] > max_weight_)
      max_weight_ = input_weights[i];
    lit_sum_ += input_weights[i];
  }

  current_max_ = max_;
}

ExplanationPbBottomLayers *
WatchedPbBottomLayersConstraint::ExplainLiteralPropagation(BooleanLiteral literal,
                                                       SolverState &state) {
  return new ExplanationPbBottomLayers(this, state, literal);
}
ExplanationPbBottomLayers *
WatchedPbBottomLayersConstraint::ExplainFailure(SolverState &state) {
  return new ExplanationPbBottomLayers(this, state);
}
WatchedPbBottomLayersConstraint::~WatchedPbBottomLayersConstraint() {
  if (encoder_ != nullptr)
    delete encoder_;
}
void WatchedPbBottomLayersConstraint::UpdateCounts(std::vector<BooleanLiteral> &lits,
                                          SolverState &state) {

  propagate_count++;

}
void WatchedPbBottomLayersConstraint::update_sum_(BooleanLiteral literal,
                                                  uint32_t weight) {
  int node_index = lit_to_node_[literal.ToPositiveInteger()];
  int diff = 0;
  auto node = nodes_[node_index];
  if (nodes_[node_index].current_sum_ < weight) {
    diff = weight - nodes_[node_index].current_sum_;
    nodes_[node_index].current_sum_ = weight;
  }
  current_sum_value_ += diff;
}

int WatchedPbBottomLayersConstraint::next_debug_index = 0;
void WatchedPbBottomLayersConstraint::EncodeBottomLayers(SolverState &state,
                                                         int nr_levels) {
  assert(nr_levels > 0);
  int lits_per_level = std::pow(2, nr_levels);
  for (int i = 0; i < inputs_.size(); i += lits_per_level) {
    std::vector<BooleanLiteral> input;
    std::vector<uint32_t> weight;
    for (int j = i; j < i + lits_per_level && j < inputs_.size(); ++j) {
      input.push_back(inputs_[j].literal);
      weight.push_back(inputs_[j].weight);
    }
    GeneralizedTotaliser totaliser(input, weight, max_);
    totaliser.Encode(state);
    assert(totaliser.root_ != nullptr);
    std::vector<WeightedLiteral> sum_nodes;
    for (int j = 0; j < totaliser.root_->counting_variables.size(); ++j) {
      sum_nodes.push_back(WeightedLiteral(totaliser.root_->counting_variables[j], totaliser.root_->counting_variables_weights[j]));
    }
    int node_index = nodes_.size();
    nodes_.push_back(SumNode(sum_nodes));
    for (auto w : sum_nodes) {
      lit_to_node_[w.literal.ToPositiveInteger()] = node_index;
    }

  }

}
} // namespace Pumpkin