//
// Created by jens on 28-10-20.
//

#include "incremental_sequential_encoder.h"
#include "../../../Engine/solver_state.h"
#include <cassert>
namespace Pumpkin {

IncrementalSequentialEncoder::IncrementalSequentialEncoder(
    std::vector<BooleanLiteral> variables, int min, int max)
    : variables_(variables), max_(max) {
  // TODO
  assert(min == 0);
  assert(max > 1);
}
IncrementalSequentialEncoder::~IncrementalSequentialEncoder() {}
std::vector<std::vector<BooleanLiteral>>
IncrementalSequentialEncoder::Encode(SolverState &state) {
  return Encode(state, variables_);
}
std::vector<std::vector<BooleanLiteral>>
IncrementalSequentialEncoder::Encode(SolverState &state,
                                     std::vector<BooleanLiteral> lits) {
  std::vector<std::vector<BooleanLiteral>> added_clauses;
  bool res = false;
  for (BooleanLiteral l : lits)
    res = AddLiteral(state, l, added_clauses) ? true : res;
  assert(res);
  for (auto c : added_clauses) {
    if (c.size() == 1)
      state.AddUnitClause(c[0]);
    else
      state.AddClause(c);
  }


  return added_clauses;
}
bool IncrementalSequentialEncoder::AddLiteral(
    SolverState &state, BooleanLiteral l,
    std::vector<std::vector<BooleanLiteral>> &added_clauses) {

  if (IsAdded(l))
    return false;
  std::vector<BooleanLiteral> current_added_lits;
  for (int i = 0; i < max_; ++i) {
    BooleanVariable v = state.CreateNewVariable();
    BooleanLiteral li(v, true);
    current_added_lits.push_back(li);
  }

  added_clauses.push_back({current_added_lits[0], ~l});

  if (!added_lits_.empty()) {
    assert(previous_added_lits_.size() == current_added_lits.size());

    for (int i = 0; i < max_; ++i)
      added_clauses.push_back({~current_added_lits[i], previous_added_lits_[i]});

    for (int i = 0; i < max_ - 1; ++i)
      added_clauses.push_back({~current_added_lits[i], previous_added_lits_[i + 1], ~added_lits_.back()});

    added_clauses.push_back({~added_lits_.back(), ~current_added_lits[max_-1]});
  }

  previous_added_lits_ = current_added_lits;
  added_lits_.push_back(l);
  return true;
}
bool IncrementalSequentialEncoder::IsAdded(BooleanLiteral lit) {
  for (BooleanLiteral l : added_lits_) {
    if (l == lit)
      return true;
  }
  return false;
}

} // namespace Pumpkin