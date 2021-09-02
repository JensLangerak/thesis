//
// Created by jens on 28-10-20.
//

#include "sequential_encoder.h"
#include "../../../Engine/solver_state.h"
namespace Pumpkin {

SequentialEncoder::SequentialEncoder(std::vector<BooleanLiteral> variables,
                                     int min, int max) : variables_(variables), min_(min), max_(max)  {}
SequentialEncoder::~SequentialEncoder() {}
std::vector<std::vector<BooleanLiteral>>
SequentialEncoder::Encode(SolverState &state) {
  int k = max_;
  auto lits = variables_;
  if (k < 0)
    throw "k should be non negative";
  if (lits.size() <= k) // always true, so nothing to encode
    return added_clauses_;
  if (k == 0) {
    for (BooleanLiteral l : lits) {
      auto l2 = ~l;
      state.AddUnitClause(l2);
      added_clauses_.push_back({~l});
    }
    return added_clauses_;
  }
  // encodes using sequential encoding
  // introduces the vars s_i_j. s_i_j is true if sum(x_[0..i]) > j
  // This leads to the following rules (here i,j >0):
  // s_0_0 = x_0 -> s_0_0 V ~x_0
  // s_0_j = F -> ~s_0_j
  // s_i_0 = max(s_i-i_0, x_i)   -> ~s_i-1_0 V s_i_0     s_i_0 V ~x_i
  // s_i_j = max(s_i-1_j, s_i-1_j-1 + x_i)
  //       -> ~s_i-1_j V s_i_j      ~s_i-1_j-1 ~x_i s_i_j
  // Force the at most: ~s_i-1_k-1 V ~x_i

  const int base_s = state.GetNumberOfVariables() + 1;
  const int n = lits.size();
  for (int i = 0; i < n*k; ++i)
    state.CreateNewVariable();

  auto s_var = [base_s, n, k](int lit_index, int sum_digit) {
    return BooleanVariable(base_s + lit_index * k + sum_digit);
  };

  // s_0_0
 added_clauses_.push_back({~lits[0], BooleanLiteral(s_var(0, 0), true)});
  // s_0_j
  for (int j = 1; j < k; j++) {
    added_clauses_.push_back({BooleanLiteral(s_var(0, j), false)});
  }
  // s_i_0
  for (int i = 1; i < n; i++) {
    added_clauses_.push_back(
        {BooleanLiteral(s_var(i - 1, 0), false), BooleanLiteral(s_var(i, 0), true)});
    added_clauses_.push_back({BooleanLiteral(s_var(i, 0), true), ~lits[i]});
  }
  // s_i_j
  for (int i = 1; i < n; i++) {
    for (int j = 1; j < k; j++) {
      added_clauses_.push_back(
          {BooleanLiteral(s_var(i - 1, j), false), BooleanLiteral(s_var(i, j), true)});
      added_clauses_.push_back({BooleanLiteral(s_var(i - 1, j - 1), false), ~lits[i],
                          BooleanLiteral(s_var(i, j), true)});
    }
  }
  // Force the constraint
  for (int i = 1; i < n; i++) {
    added_clauses_.push_back({BooleanLiteral(s_var(i - 1, k - 1), false), ~lits[i]});
  }
  for (auto c : added_clauses_) {
    if (c.size() == 1)
      state.AddUnitClause(c[0]);
    else
    state.AddClause(c);
  }
  encoding_added_ = true;
  return added_clauses_;
}
IEncoder<CardinalityConstraint> *
SequentialEncoder::Factory::CallConstructor(CardinalityConstraint &constraint) {
  return new SequentialEncoder(constraint.literals, constraint.min, constraint.max);
}
}
