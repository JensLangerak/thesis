//
// Created by jens on 05-10-20.
//

#include "totaliser_encoder.h"
namespace simple_sat_solver::sat {
TotaliserEncoder::TotaliserEncoder(SatProblem *sat, std::vector<Lit> variables,
                                   int min, int max)
    : sat_(sat), variables_(variables), min_(min), max_(max),
      root_(nullptr){}
void TotaliserEncoder::Encode(SatProblem &sat, std::vector<Lit> variables,
                              int min, int max) {
  TotaliserEncoder encoder(&sat, variables, min, max);
  encoder.root_ = encoder.CreateTree(variables);
  encoder.SetMin();
  encoder.SetMax();
}
TotaliserEncoder::Node *TotaliserEncoder::CreateTree(std::vector<Lit> variables) {
  Node *n = new Node();
  n->index = variables.size();
  n->variables = variables;

  if (n->index == 1) {
    n->counting_variables = n->variables;
  } else {

    n->counting_variables = std::vector<Lit>();
    n->counting_variables.reserve(variables.size());
    for (int i = 0; i < variables.size(); i++) {
      n->counting_variables.emplace_back(sat_->AddNewVar());
    }

    int m = n->index / 2;
    std::vector<Lit> variables_left(variables.begin(), variables.begin() + m);
    std::vector<Lit> variables_right(variables.begin() + m, variables.end());

    n->left = CreateTree(variables_left);
    n->right = CreateTree(variables_right);
    if (n->left->variables.size() + n->right->variables.size() !=
        n->variables.size())
      throw "Tree not correct";

    // count a = 0 (index -1)
    for (int b = 0; b < n->right->counting_variables.size(); ++b) {
      int d = b;
      sat_->AddClause(
          {~n->right->counting_variables[b], n->counting_variables[d]});
      if (b < n->right->counting_variables.size() - 1)
      sat_->AddClause({n->left->counting_variables[0],
                       n->right->counting_variables[b + 1],
                       ~n->counting_variables[d + 1]});
      else {
        sat_->AddClause({n->left->counting_variables[0], ~n->counting_variables[d+1]});
      }
    }

    // count b = 0 (index -1)
    for (int a = 0; a < n->left->counting_variables.size(); ++a) {
      int d = a;
      sat_->AddClause(
          {~n->left->counting_variables[a], n->counting_variables[d]});
      if (a < n->left->counting_variables.size() - 1)
      sat_->AddClause({n->left->counting_variables[a],
                       n->right->counting_variables[0],
                       ~n->counting_variables[d + 1]});
      else
      sat_->AddClause({n->right->counting_variables[0], ~n->counting_variables[d+1]});
    }

    for (int a = 0; a < n->left->counting_variables.size(); ++a) {
      for (int b = 0; b < n->right->counting_variables.size(); ++b) {
        int d = (a + 1) + (b + 1) - 1;
        sat_->AddClause({~n->left->counting_variables[a],
                         ~n->right->counting_variables[b],
                         n->counting_variables[d]});
        bool max_a = a == n->left->counting_variables.size() - 1;
        bool max_b = b == n->right->counting_variables.size() - 1;
        if (!(max_a || max_b))
          sat_->AddClause({n->left->counting_variables[a + 1],
                           n->right->counting_variables[b + 1],
                           ~n->counting_variables[d + 1]});
        else if (!max_b)
          sat_->AddClause({n->right->counting_variables[b + 1],
                           ~n->counting_variables[d + 1]});
        else if (!max_a)
          sat_->AddClause({n->left->counting_variables[a + 1],
                           ~n->counting_variables[d + 1]});
      }
    }
  }
  return n;
}
TotaliserEncoder::~TotaliserEncoder() { delete root_; }
void TotaliserEncoder::SetMin() {
  if (min_ < 1)
    return;
  if (min_ > max_)
    throw "min is larger than the max";
  if (min_ > (int) variables_.size()) {
    sat_->AddClause({}); // always false
    return;
  }
  for (int i = 0; i<min_; ++i) {
    sat_->AddClause({Lit(root_->counting_variables[i])});
  }

}
void TotaliserEncoder::SetMax() {
  if (max_ >= (int) variables_.size())
    return;
  if (min_ > max_)
    throw "min is larger than the max";
  if (max_ < 0)
    throw "max is negative";
  // set the value max_ + 1 to false. This has index max_
  for (int i = max_; i< root_->counting_variables.size(); ++i) {
    sat_->AddClause({Lit(~root_->counting_variables[i])});
  }

}
TotaliserEncoder::Node::~Node() {
  if (left!= nullptr)
  delete left;
  if (left != nullptr)
  delete right;
}
}; // namespace simple_sat_solver::sat
