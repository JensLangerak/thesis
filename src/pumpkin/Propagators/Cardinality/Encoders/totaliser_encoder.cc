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
TotaliserEncoder::Node2 *
TotaliserEncoder::CreateTree(std::vector<BooleanLiteral> variables) {
  Node2 *n = new Node2();
  n->index = variables.size();
  n->variables = variables;

  if (n->index == 1) {
    n->counting_variables = n->variables;
  } else {

    n->counting_variables = std::vector<BooleanLiteral>();
    n->counting_variables.reserve(variables.size());
    for (int i = 0; i < variables.size(); i++) {
      n->counting_variables.push_back(
          BooleanLiteral(solver_state_->CreateNewVariable(), true));
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

    // count a = 0 (index -1)
    for (int b = 0; b < n->right->counting_variables.size(); ++b) {
      int d = b;
      std::vector<BooleanLiteral> clause = {~n->right->counting_variables[b],
                                            n->counting_variables[d]};
      solver_state_->AddClause(clause);
      if (b < n->right->counting_variables.size() - 1) {
        clause = {n->left->counting_variables[0],
                  n->right->counting_variables[b + 1],
                  ~n->counting_variables[d + 1]};
        added_clauses_.push_back(clause);
        solver_state_->AddClause(clause);
      } else {
        clause = {n->left->counting_variables[0],
                  ~n->counting_variables[d + 1]};
        added_clauses_.push_back(clause);
        solver_state_->AddClause(clause);
      }
    }

    // count b = 0 (index -1)
    for (int a = 0; a < n->left->counting_variables.size(); ++a) {
      int d = a;
      std::vector<BooleanLiteral> clause = {~n->left->counting_variables[a],
                                            n->counting_variables[d]};
      solver_state_->AddClause(clause);
      if (a < n->left->counting_variables.size() - 1) {
        clause = {n->left->counting_variables[a],
                  n->right->counting_variables[0],
                  ~n->counting_variables[d + 1]};
        added_clauses_.push_back(clause);
        solver_state_->AddClause(clause);
      } else {
        clause = {n->right->counting_variables[0],
                  ~n->counting_variables[d + 1]};
        added_clauses_.push_back(clause);
        solver_state_->AddClause(clause);
      }
    }

    for (int a = 0; a < n->left->counting_variables.size(); ++a) {
      for (int b = 0; b < n->right->counting_variables.size(); ++b) {
        int d = (a + 1) + (b + 1) - 1;
        std::vector<BooleanLiteral> clause = {~n->left->counting_variables[a],
                                              ~n->right->counting_variables[b],
                                              n->counting_variables[d]};
        added_clauses_.push_back(clause);
        solver_state_->AddClause(clause);
        bool max_a = a == n->left->counting_variables.size() - 1;
        bool max_b = b == n->right->counting_variables.size() - 1;
        if (!(max_a || max_b)) {
          clause = {n->left->counting_variables[a + 1],
                    n->right->counting_variables[b + 1],
                    ~n->counting_variables[d + 1]};
          added_clauses_.push_back(clause);
          solver_state_->AddClause(clause);
        } else if (!max_b) {
          clause = {n->right->counting_variables[b + 1],
                    ~n->counting_variables[d + 1]};
          added_clauses_.push_back(clause);
          solver_state_->AddClause(clause);
        } else if (!max_a) {
          clause = {n->left->counting_variables[a + 1],
                    ~n->counting_variables[d + 1]};
          added_clauses_.push_back(clause);
          solver_state_->AddClause(clause);
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
  return added_clauses_;
}
TotaliserEncoder::Node2::~Node2() {
  if (left != nullptr)
    delete left;
  if (left != nullptr)
    delete right;
}
}; // namespace Pumpkin
