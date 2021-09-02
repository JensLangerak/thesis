//
// Created by jens on 06-07-21.
//

#include "watched_bottom_layers_constraint.h"
#include "../Encoders/generalized_totaliser.h"
#include "explanation_bottom_layers_constraint.h"
#include <cmath>
#include <unordered_set>
#include "../../../Engine/solver_state.h"
namespace Pumpkin {

WatchedBottomLayersConstraint::WatchedBottomLayersConstraint(
    PseudoBooleanConstraint &constraint) : current_sum_value_(0), max_(constraint.right_hand_side){
  auto inputs = constraint.literals;
  auto input_weights = constraint.coefficients;
  assert(inputs.size() == input_weights.size());
  inputs_.reserve(inputs.size());
  for (int i = 0; i < inputs.size(); ++i) {
    inputs_.push_back(WeightedLiteral(inputs[i], input_weights[i]));
    lit_to_input_index_[inputs[i]] = i;
    if (input_weights[i] > max_weight_)
      max_weight_ = input_weights[i];
    lit_sum_ += input_weights[i];
  }

  current_max_ = max_;

}
ExplanationGeneric *WatchedBottomLayersConstraint::ExplainFailure(
    SolverState &state, ExplanationDynamicConstraint *explanation) {
  ExplanationBottomLayersConstraint::InitExplanationBottomLayers(
      this, state, explanation);
  return explanation;
}
ExplanationGeneric *WatchedBottomLayersConstraint::ExplainLiteralPropagation(
    BooleanLiteral literal, SolverState &state,
    ExplanationDynamicConstraint *explanation) {
  ExplanationBottomLayersConstraint::InitExplanationBottomLayers(
      this, state, literal, explanation);
  return explanation;
}
void WatchedBottomLayersConstraint::UpdateSum(BooleanLiteral literal,
                                              uint32_t weight, SolverState& state) {
  int node_index =  lit_to_node_index_[literal];
  int diff = 0;
  auto node = nodes_[node_index];
  if (nodes_[node_index].current_sum_ < weight) {
    diff = weight - nodes_[node_index].current_sum_;
    nodes_[node_index].current_sum_ = weight;
    if (update_nodes_decision_boundaries_.empty() || update_nodes_decision_boundaries_.top().first < state.GetCurrentDecisionLevel()) {
      update_nodes_decision_boundaries_.push(std::pair(state.GetCurrentDecisionLevel(), updated_nodes_index_.size()));
    }
    updated_nodes_index_.push(node_index);
  }
  current_sum_value_ += diff;
}
void WatchedBottomLayersConstraint::EncodeBottomLayers(SolverState &state,
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
      std::vector<WeightedLiteral> input_lits;
      for (int j = 0; j < input.size(); ++j) {
        input_lits.push_back(WeightedLiteral(input[j], weight[j]));
      }
      int node_index = nodes_.size();
      nodes_.push_back(SumNode(sum_nodes, input_lits));
      for (auto w : sum_nodes) {
        lit_to_node_index_[w.literal] = node_index;
      }

    }
}
void WatchedBottomLayersConstraint::Synchronise(SolverState &state) {
  while ((!decision_level_sums_.empty()) &&
      decision_level_sums_.top().level > state.GetCurrentDecisionLevel()) {
    decision_level_sums_.pop();
  }
  if (decision_level_sums_.empty())
    current_sum_value_ = 0;
  else
    current_sum_value_ = decision_level_sums_.top().sum_value;

  int stop_size = updated_nodes_index_.size();
  while ((!update_nodes_decision_boundaries_.empty()) && update_nodes_decision_boundaries_.top().first > state.GetCurrentDecisionLevel()) {
    stop_size = update_nodes_decision_boundaries_.top().second;
    update_nodes_decision_boundaries_.pop();
  }

  std::unordered_set<int> handled_nodes;
  while(updated_nodes_index_.size() > stop_size) {
    int index = updated_nodes_index_.top();
    updated_nodes_index_.pop();
    if (handled_nodes.count(index) == 0) {
      handled_nodes.insert(index);
      int sum = 0;
      auto node = nodes_[index];
      for (auto input : node.sum_literals) {
        if (state.assignments_.IsAssignedTrue(input.literal) && sum < input.weight) {
          sum = input.weight;
        }
      }
      nodes_[index].current_sum_ = sum;
    }
  }


}
void WatchedBottomLayersConstraint::UpdateDecisionLevelSum(SolverState &state) {
  if (decision_level_sums_.empty() ||
      decision_level_sums_.top().level < state.GetCurrentDecisionLevel()) {
    decision_level_sums_.push(DecisionLevelCount(
        state.GetCurrentDecisionLevel(), current_sum_value_));
  } else {
    assert(decision_level_sums_.top().level == state.GetCurrentDecisionLevel());
    decision_level_sums_.top().sum_value = current_sum_value_;
  }
}
bool WatchedBottomLayersConstraint::CountCorrect(SolverState &state,
                                                 BooleanLiteral literal) {
  int real_count = 0;
  int max_position = state.assignments_.GetTrailPosition(literal.Variable());
  for (auto lit : inputs_) {
    if (state.assignments_.IsAssignedTrue(lit.literal) && max_position >= state.assignments_.GetTrailPosition(lit.literal.Variable())) {
      real_count += lit.weight;
    }
  }

  int sum_nodes = 0;
  int sum_nodes_max = 0;
  for (SumNode n : nodes_) {
    int v = 0;
    int v_max = 0;
    for (auto lit : n.sum_literals) {
      if (state.assignments_.IsAssignedTrue(lit.literal)) {
        if (lit.weight > v_max)
          v_max = lit.weight;
      }
      if (state.assignments_.IsAssignedTrue(lit.literal) && max_position >= state.assignments_.GetTrailPosition(lit.literal.Variable())) {
        if (lit.weight > v)
          v = lit.weight;
      }
    }
    int s = 0;
    for (auto lit : n.inputs_debug) {
      if (state.assignments_.IsAssignedTrue(lit.literal) && max_position >= state.assignments_.GetTrailPosition(lit.literal.Variable())) {
        s+= lit.weight;
      }
    }
    if (s>v ) {
      bool t = state.propagator_clausal_.IsPropagationComplete(state);
//      int i = state.assignments_.GetAssignmentLevel(n.inputs_debug[0].literal);
//      int i2 = state.assignments_.GetTrailPosition(n.inputs_debug[0].literal.Variable());
//      int i3 = state.GetCurrentDecisionLevel();
     assert(t);
    }
    if (v != n.current_sum_) {
      bool a0 = state.assignments_.IsAssigned(n.sum_literals[0].literal);
      bool a1 = state.assignments_.IsAssignedTrue(n.sum_literals[1].literal);
      bool b1 = state.assignments_.IsAssigned(n.sum_literals[0].literal);
      bool b2 = state.assignments_.IsAssignedTrue(n.sum_literals[1].literal);
      int test = 2;


    }
    assert(s <= v_max);
    assert(v == n.current_sum_);
    sum_nodes += v;
    sum_nodes_max += v_max;
  }
  assert(sum_nodes_max >= real_count);
  assert(current_sum_value_ == sum_nodes);



  return true;
}
}
