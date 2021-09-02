//
// Created by jens on 05-10-20.
//

#include "totaliser_encoder.h"
#include "../sat_problem.h"
#include <cassert>
namespace simple_sat_solver::sat {
TotaliserEncoder::TotaliserEncoder(SatProblem *sat, std::vector<Lit> variables,
                                   int min, int max)
    : sat_(sat), variables_(variables), min_(min), max_(max),
      root_(nullptr){}
void TotaliserEncoder::Encode(SatProblem &sat, std::vector<Lit> variables,
                              int min, int max) {
  if (variables.size() < 1) {
    return;
  }
  TotaliserEncoder  encoder =  TotaliserEncoder(&sat, variables, min, max);
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

    for(int a = 0; a <= n->left->counting_variables.size(); ++a) {
      for (int b = 0; b <= n->right->counting_variables.size(); ++b) {
        int d = a + b;

        int index_a = a - 1;
        int index_b = b - 1;
        int index_d = d - 1;

        std::vector<Lit> c1;
        if (index_a >= 0)
          c1.push_back(~n->left->counting_variables[index_a]);
        if (index_b >= 0)
          c1.push_back(~n->right->counting_variables[index_b]);
        if (index_d >= 0) {
          c1.push_back(n->counting_variables[index_d]);
          sat_->AddClause(c1);
        }

        std::vector<Lit> c2;
        if (index_a + 1 < n->left->counting_variables.size())
          c2.push_back(n->left->counting_variables[index_a + 1]);
        if (index_b + 1 < n->right->counting_variables.size())
          c2.push_back(n->right->counting_variables[index_b + 1]);
        if (index_d + 1 < n->counting_variables.size()) {
          c2.push_back(~n->counting_variables[index_d + 1]);
          sat_->AddClause(c2);
        }
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
