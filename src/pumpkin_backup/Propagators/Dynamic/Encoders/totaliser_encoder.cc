//
// Created by jens on 05-10-20.
//

#include "../../../Engine/solver_state.h"
#include "totaliser_encoder.h"
namespace Pumpkin {
TotaliserEncoder::TotaliserEncoder(
                                   std::vector<BooleanLiteral> variables,
                                   int min, int max)
    : solver_state_(NULL), variables_(variables), min_(min), max_(max),
      root_(nullptr) {}
//std::vector<std::vector<BooleanLiteral>>
//TotaliserEncoder::Encode(SolverState &solver_state,
//                         std::vector<BooleanLiteral> variables, int min,
//                         int max) {
//  assert(min == 0); // TODO implement
//  TotaliserEncoder encoder(variables, min, max);
//  return encoder.Encode(solver_state);
//}
TotaliserEncoder::Node *
TotaliserEncoder::CreateTree(std::vector<BooleanLiteral> variables) {
    return CreateTree(variables, false);
}
TotaliserEncoder::Node *
TotaliserEncoder::CreateTree(std::vector<BooleanLiteral> variables, bool use_set_sum_lits) {
  Node *n = new Node();
  n->index = variables.size();
  n->variables = variables;

  if (n->index == 1) {
    n->counting_variables = n->variables;
  } else {

    if (use_set_sum_lits) {
      n->counting_variables = potential_sum_literals_;
    } else {
      n->counting_variables = std::vector<BooleanLiteral>();
      n->counting_variables.reserve(variables.size());
      for (int i = 0; i < variables.size(); i++) {
        n->counting_variables.push_back(
            BooleanLiteral(solver_state_->CreateNewVariable(), true));
      }
    }

    int m = n->index / 2;
    std::vector<BooleanLiteral> variables_left(variables.begin(),
                                               variables.begin() + m);
    std::vector<BooleanLiteral> variables_right(variables.begin() + m,
                                                variables.end());

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
        if (index_d >= n->counting_variables.size())
          continue;

        std::vector<BooleanLiteral> c1;
        if (index_a >= 0)
          c1.push_back(~n->left->counting_variables[index_a]);
        if (index_b >= 0)
          c1.push_back(~n->right->counting_variables[index_b]);
        if (index_d >= 0) {
          c1.push_back(n->counting_variables[index_d]);
          solver_state_->AddClause(c1);
        }

        if (index_d + 1>= n->counting_variables.size())
          continue;
        std::vector<BooleanLiteral> c2;
        if (index_a + 1 < n->left->counting_variables.size())
          c2.push_back(n->left->counting_variables[index_a + 1]);
        if (index_b + 1 < n->right->counting_variables.size())
          c2.push_back(n->right->counting_variables[index_b + 1]);
        if (index_d + 1 < n->counting_variables.size()) {
          c2.push_back(~n->counting_variables[index_d + 1]);
          solver_state_->AddClause(c2);
        }
      }
    }
  }
  return n;
}
TotaliserEncoder::~TotaliserEncoder() {
  delete root_; }
void TotaliserEncoder::SetMin() {
  if (min_ < 1)
    return;
  if (min_ > max_)
    throw "min is larger than the max";
  if (min_ > (int)variables_.size()) {
    std::vector<BooleanLiteral> clause;
    added_clauses_.push_back(clause);
    solver_state_->AddClause(clause);
    return;
  }
  for (int i = 0; i < min_; ++i) {
    BooleanLiteral clause = root_->counting_variables[i];
    added_clauses_.push_back({clause});
    solver_state_->AddUnitClause(clause);
    //    solver_state_->InsertPropagatedLiteral(clause, NULL, NULL, 0);
  }
}
void TotaliserEncoder::SetMax() {
  if (max_ >= (int)variables_.size())
    return;
  if (min_ > max_)
    throw "min is larger than the max";
  if (max_ < 0)
    throw "max is negative";
  // set the value max_ + 1 to false. This has index max_
  for (int i = max_; i < root_->counting_variables.size(); ++i) {
    BooleanLiteral clause = ~root_->counting_variables[i];
    added_clauses_.push_back({clause});
    solver_state_->AddUnitClause(clause);
    //    solver_state_->InsertPropagatedLiteral(clause, NULL, NULL, 0);
  }
}
std::vector<std::vector<BooleanLiteral>>
TotaliserEncoder::Encode(SolverState &state) {
  solver_state_ = &state;
  root_ = CreateTree(variables_);
  SetMin();
  SetMax();
  encoding_added_ = true;
  return added_clauses_;
}
//void TotaliserEncoder::SetSumLiterals(std::vector<BooleanLiteral> sum_lits) {
////  assert(potential_sum_literals_.size() == variables_.size());
//  potential_sum_literals_ = sum_lits;
//}
TotaliserEncoder::Node::~Node() {
  if (left != nullptr)
    delete left;
  if (left != nullptr)
    delete right;
}
IEncoder<CardinalityConstraint> *
TotaliserEncoder::Factory::CallConstructor(CardinalityConstraint& constraint) {
  return new TotaliserEncoder(constraint.literals, constraint.min, constraint.max);
}
}; // namespace Pumpkin
