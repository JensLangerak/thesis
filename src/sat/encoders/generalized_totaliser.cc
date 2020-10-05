//
// Created by jens on 05-10-20.
//

#include "generalized_totaliser.h"
namespace simple_sat_solver::sat {
GeneralizedTotaliser::GeneralizedTotaliser(SatProblem *sat,
                                           std::vector<Lit> variables,
                                           std::vector<int> weights, int max)
    : sat_(sat), variables_(variables), max_(max), weights_(weights),
      root_(nullptr) {}
void GeneralizedTotaliser::Encode(SatProblem &sat, std::vector<Lit> variables,
                                  std::vector<int> weights, int max) {
  GeneralizedTotaliser encoder(&sat, variables, weights, max);
  encoder.root_ = encoder.CreateTree(variables, weights);
  encoder.SetMax();
}
GeneralizedTotaliser::Node *
GeneralizedTotaliser::CreateTree(std::vector<Lit> variables,
                                 std::vector<int> weights) {
  Node *n = new Node();
  n->index = variables.size();
  n->variables = variables;
  n->variables_weights = weights;
  n->max = 0;

  if (n->index == 1) {
    n->counting_variables = n->variables;
    n->counting_variables_weights = n->variables_weights;
    n->max = n->variables_weights[0];
  } else {
    n->counting_variables = std::vector<Lit>();

    int m = n->index / 2;
    std::vector<Lit> variables_left(variables.begin(), variables.begin() + m);
    std::vector<Lit> variables_right(variables.begin() + m, variables.end());

    std::vector<int> weights_left(weights.begin(), weights.begin() + m);
    std::vector<int> weights_right(weights.begin() + m, weights.end());

    n->left = CreateTree(variables_left, weights_left);
    n->right = CreateTree(variables_right, weights_right);
    if (n->left->variables.size() + n->right->variables.size() !=
        n->variables.size())
      throw "Tree not correct";

    int max = n->left->max + n->right->max;
    n->values_map = std::vector<int>(max + 1, -1);

    for (int q = 0; q < n->left->counting_variables.size(); ++q) {
      for (int r = 0; r < n->right->counting_variables.size(); ++r) {
        int q_v = n->left->counting_variables_weights[q];
        int r_v = n->right->counting_variables_weights[r];
        int p_w = q_v + r_v;
        int p = GetValueIndex(n, p_w);
        sat_->AddClause({~n->left->counting_variables[q],
                         ~Lit(n->right->counting_variables[r]),
                         n->counting_variables[p]});
      }
    }
    for (int q = 0; q < n->left->counting_variables.size(); ++q) {
      int q_v = n->left->counting_variables_weights[q];
      int p_w = q_v;
      int p = GetValueIndex(n, p_w);
      sat_->AddClause(
          {~n->left->counting_variables[q], n->counting_variables[p]});
    }
    for (int r = 0; r < n->right->counting_variables.size(); ++r) {
      int r_v = n->right->counting_variables_weights[r];
      int p_w = r_v;
      int p = GetValueIndex(n, p_w);
      sat_->AddClause(
          {~n->right->counting_variables[r], n->counting_variables[p]});
    }
  }
  return n;
}
GeneralizedTotaliser::~GeneralizedTotaliser() { delete root_; }
void GeneralizedTotaliser::SetMax() {
  if (max_ < 0)
    throw "max is negative";
  // set the value max_ + 1 to false. This has index max_
  for (int i = 0; i < root_->counting_variables.size(); ++i) {
    if (root_->counting_variables_weights[i] > max_)
      sat_->AddClause({Lit(~root_->counting_variables[i])});
  }
}

int GeneralizedTotaliser::GetValueIndex(GeneralizedTotaliser::Node *node,
                                        int value) {
  if (node->values_map[value] == -1) {
    node->values_map[value] = node->counting_variables.size();
    node->counting_variables.push_back(sat_->AddNewVar());
    node->counting_variables_weights.push_back(value);
    if (value > node->max)
      node->max = value;
  }
  return node->values_map[value];
}
GeneralizedTotaliser::Node::~Node() {
  if (left != nullptr)
    delete left;
  if (left != nullptr)
    delete right;
}
} // namespace simple_sat_solver::sat