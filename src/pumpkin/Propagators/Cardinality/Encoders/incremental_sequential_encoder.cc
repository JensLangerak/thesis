//
// Created by jens on 28-10-20.
//

#include "incremental_sequential_encoder.h"
#include "../../../Engine/solver_state.h"
#include <cassert>
#include <iostream>
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
    res = AddLiteral(state, l, added_clauses) ? true
                                              : res; // TODO something faster?
  if (!res) { // currently propagation is done in two phases, first the encoding
              // is added, if encoding is not updated, then propagate
    return std::vector<std::vector<BooleanLiteral>>();
  }
  for (auto c : added_clauses) {
    if (c.size() == 1)
      state.AddUnitClause(c[0]);
    else
      state.AddClause(c);
  }

  //  std::vector<BooleanLiteral> partial_propagate_clause;
  //  // TODO move to propagate
  //  partial_propagate_clause.push_back(~previous_added_lits_[0]);
  //  partial_propagate_clause.push_back(previous_added_lits_[1]);
  //  partial_propagate_clause.push_back(~added_lits_.back());
  //  this->paritial_propagate_clause_ = partial_propagate_clause;
  return added_clauses;
}
bool IncrementalSequentialEncoder::AddLiteral(
    SolverState &state, BooleanLiteral l,
    std::vector<std::vector<BooleanLiteral>> &added_clauses) {

  if (IsAdded(l)) // TODO assert? function is slow
    return false;
  if (previous_added_lits_.empty()) {
    for (int i = 0; i < max_; ++i) {
      BooleanVariable v = state.CreateNewVariable();
      BooleanLiteral li(v, true);
      previous_added_lits_.push_back(li);
    }
    hist.push_back(previous_added_lits_);
  }
  std::vector<std::vector<bool>> hist_assigned;
  std::vector<std::vector<bool>> hist_value;
  for (auto s : hist) {

    std::vector<bool> assigned;
    std::vector<bool> value;
    for (auto l : s) {
      assigned.push_back(state.assignments_.IsAssigned(l));
      value.push_back(state.assignments_.IsAssignedTrue(l));
    }
    hist_assigned.push_back(assigned);
    hist_value.push_back(value);
  }
  std::vector<bool> vars_assigned;
  std::vector<bool> vars_value;
  for (BooleanLiteral l : added_lits_) {
    vars_assigned.push_back(state.assignments_.IsAssigned(l));
    vars_value.push_back(state.assignments_.IsAssignedTrue(l));
  }

  std::vector<BooleanLiteral> current_added_lits;
  for (int i = 0; i < max_; ++i) {
    BooleanVariable v = state.CreateNewVariable();
    BooleanLiteral li(v, true);
    current_added_lits.push_back(li);
  }

  added_clauses.push_back({previous_added_lits_[0], ~l});

  //  if (!added_lits_.empty()) {
  assert(previous_added_lits_.size() == current_added_lits.size());

  for (int i = 0; i < max_; ++i) {
    added_clauses.push_back({~current_added_lits[i], previous_added_lits_[i]});
  }

  for (int i = 0; i < max_ - 1; ++i) {
    added_clauses.push_back(
        {~current_added_lits[i], previous_added_lits_[i + 1], ~l});
  }
  added_clauses.push_back({~l, ~current_added_lits[max_ - 1]});
  //  }

  previous_added_lits_ = current_added_lits;
  hist.push_back(current_added_lits);
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
std::vector<std::vector<BooleanLiteral>>
IncrementalSequentialEncoder::Propagate(
    SolverState &state, std::vector<BooleanLiteral> reason,
    std::vector<BooleanLiteral> propagated_values) {
  std::vector<std::vector<BooleanLiteral>> added_clauses;
  for (BooleanLiteral l : propagated_values) {
    std::vector<bool> vars_assigned;
    std::vector<bool> vars_value;
    for (BooleanLiteral l : added_lits_) {
      vars_assigned.push_back(state.assignments_.IsAssigned(l));
      vars_value.push_back(state.assignments_.IsAssignedTrue(l));
    }

    std::vector<std::vector<bool>> hist_assigned;
    std::vector<std::vector<bool>> hist_value;
    for (auto s : hist) {

      std::vector<bool> assigned;
      std::vector<bool> value;
      for (auto l : s) {
        assigned.push_back(state.assignments_.IsAssigned(l));
        value.push_back(state.assignments_.IsAssignedTrue(l));
      }
      hist_assigned.push_back(assigned);
      hist_value.push_back(value);
    }

    PrintState(state);
    assert(!state.assignments_.IsAssigned(l));

    assert(!IsAdded(l));
    std::vector<BooleanLiteral> learned_clause = {previous_added_lits_[0], ~l};
    added_clauses.push_back(learned_clause);
    TwoWatchedClause *clause =
        state.propagator_clausal_.clause_database_.AddPermanentClause(
            learned_clause, state);
  }
  return added_clauses;
}
void IncrementalSequentialEncoder::PrintInfo() {
  std::cout << "Info: " << added_lits_.size() << " / "
            << this->variables_.size() << std::endl;
}
void IncrementalSequentialEncoder::PrintState(SolverState &state) {
  return;
  std::cout << "max: " << max_ << std::endl;
  for (int i = 0; i < hist.size(); ++i) {
    std::cout << i << ": " ;
    for (BooleanLiteral l : hist[i]) {
      if (state.assignments_.IsAssigned(l)) {

        std::cout << state.assignments_.GetAssignmentLevel(l.Variable());
        if (state.assignments_.IsDecision(l.Variable())) {
          std::cout << (state.assignments_.IsAssignedTrue(l.Variable()) ? "T "
                                                                       : "F ");
        } else {
          std::cout << (state.assignments_.IsAssignedTrue(l.Variable()) ? "t "
                                                                       : "f ");
        }
      } else {
        std::cout << "_ ";
      }
    }
    std::cout << std::endl;
    if (i < added_lits_.size()) {
      BooleanLiteral l = added_lits_[i];
      if (state.assignments_.IsAssigned(l) && (!state.propagator_clausal_.next_position_on_trail_to_propagate_it.IsPastTrail() && state.assignments_.GetTrailPosition(l.Variable()) > state.assignments_.GetTrailPosition(state.propagator_clausal_.next_position_on_trail_to_propagate_it.GetData().Variable())))
        std::cout << "TR_C " ;
      if (state.assignments_.IsAssigned(l) && state.assignments_.GetTrailPosition(l.Variable()) > state.assignments_.GetTrailPosition(state.propagator_cardinality_.next_position_on_trail_to_propagate_it.GetData().Variable()))
        std::cout << "TR_P " ;
      if (!state.assignments_.IsAssigned(l))
        std::cout << "_ " << std::endl;
      else if (state.assignments_.IsDecision(l.Variable())) {
        std::cout << state.assignments_.GetAssignmentLevel(l.Variable());
        std::cout << (state.assignments_.IsAssignedTrue(l.Variable()) ? "T "
                                                                      : "f ")
                  << std::endl;
      }else {
        std::cout << state.assignments_.GetAssignmentLevel(l.Variable());
        std::cout << (state.assignments_.IsAssignedTrue(l.Variable()) ? "t "
                                                                      : "f ")
                  << std::endl;
      }
    }
  }
}
void IncrementalSequentialEncoder::SetSumLiterals(std::vector<BooleanLiteral> sum_lits) {
  if (!previous_added_lits_.empty())
    throw "sums already set";
  assert(sum_lits.size() == max_);
  previous_added_lits_ = sum_lits;
  hist.push_back(previous_added_lits_);
}

} // namespace Pumpkin