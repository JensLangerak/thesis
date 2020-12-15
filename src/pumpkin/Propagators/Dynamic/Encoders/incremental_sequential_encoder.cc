//
// Created by jens on 28-10-20.
//

#include "incremental_sequential_encoder.h"
#include "../../../../logger/logger.h"
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
  encoding_added_ = true;
  return Encode(state, variables_);
}
std::vector<std::vector<BooleanLiteral>>
IncrementalSequentialEncoder::Encode(SolverState &state,
                                     std::vector<BooleanLiteral> lits) {
  partial_added_ = true;
  std::vector<std::vector<BooleanLiteral>> added_clauses;
  bool res = false;
  int current_lits = added_lits_.size();
  for (BooleanLiteral l : lits)
    res = AddLiteral(state, l, added_clauses) ? true
                                              : res; // TODO something faster?
  if (!res) { // currently propagation is done in two phases, first the encoding
              // is added, if encoding is not updated, then propagate
    return std::vector<std::vector<BooleanLiteral>>();
  }
  encoding_added_ = added_lits_.size() == variables_.size();
  int new_lits = added_lits_.size();
  simple_sat_solver::logger::Logger::Log2("Add encoding: ID " + std::to_string(log_id_) + " added vars " + std::to_string(current_lits) + " " + std::to_string(new_lits) + " " +std::to_string(variables_.size()));
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
    sum_lits = previous_added_lits_;
    hist.push_back(previous_added_lits_);
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
  added_lits_[l.ToPositiveInteger()] = true;
  added_lit_hist_.push_back(l);
  return true;
}
bool IncrementalSequentialEncoder::IsAdded(BooleanLiteral lit) {
  auto it = added_lits_.find(lit.ToPositiveInteger());
  if (it == added_lits_.end()) {
    return false;
  }
  return it->second;
}
std::vector<std::vector<BooleanLiteral>>
IncrementalSequentialEncoder::Propagate(
    SolverState &state, std::vector<BooleanLiteral> reason,
    std::vector<BooleanLiteral> propagated_values) {
  std::vector<std::vector<BooleanLiteral>> added_clauses;
  for (BooleanLiteral l : propagated_values) {
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
//  for (int i = 0; i < hist.size(); ++i) {
//    std::cout << i << ": " ;
//    for (BooleanLiteral l : hist[i]) {
//      if (state.assignments_.IsAssigned(l)) {
//
//        std::cout << state.assignments_.GetAssignmentLevel(l.Variable());
//        if (state.assignments_.IsDecision(l.Variable())) {
//          std::cout << (state.assignments_.IsAssignedTrue(l.Variable()) ? "T "
//                                                                       : "F ");
//        } else {
//          std::cout << (state.assignments_.IsAssignedTrue(l.Variable()) ? "t "
//                                                                       : "f ");
//        }
//      } else {
//        std::cout << "_ ";
//      }
//    }
//    std::cout << std::endl;
//    if (i < added_lits_.size()) {
//      BooleanLiteral l = added_lit_hist_[i];
//      if (state.assignments_.IsAssigned(l) && (!state.propagator_clausal_.next_position_on_trail_to_propagate_it.IsPastTrail() && state.assignments_.GetTrailPosition(l.Variable()) > state.assignments_.GetTrailPosition(state.propagator_clausal_.next_position_on_trail_to_propagate_it.GetData().Variable())))
//        std::cout << "TR_C " ;
//      if (state.assignments_.IsAssigned(l) && state.assignments_.GetTrailPosition(l.Variable()) > state.assignments_.GetTrailPosition(state.propagator_cardinality_.next_position_on_trail_to_propagate_it.GetData().Variable()))
//        std::cout << "TR_P " ;
//      if (!state.assignments_.IsAssigned(l))
//        std::cout << "_ " << std::endl;
//      else if (state.assignments_.IsDecision(l.Variable())) {
//        std::cout << state.assignments_.GetAssignmentLevel(l.Variable());
//        std::cout << (state.assignments_.IsAssignedTrue(l.Variable()) ? "T "
//                                                                      : "f ")
//                  << std::endl;
//      }else {
//        std::cout << state.assignments_.GetAssignmentLevel(l.Variable());
//        std::cout << (state.assignments_.IsAssignedTrue(l.Variable()) ? "t "
//                                                                      : "f ")
//                  << std::endl;
//      }
//    }
//  }
}
//void IncrementalSequentialEncoder::SetSumLiterals(std::vector<BooleanLiteral> sum_lits) {
//  if (!previous_added_lits_.empty())
//    throw "sums already set";
//  assert(sum_lits.size() == max_);
//  previous_added_lits_ = sum_lits;
//  sum_lits = sum_lits;
//  hist.push_back(previous_added_lits_);
//}
enum class LitValue {True, False, Unknown};
void IncrementalSequentialEncoder::DebugInfo(SolverState &state) {
  std::vector<LitValue> var_values;
  for (BooleanLiteral l : added_lit_hist_) {
    if (state.assignments_.IsAssignedTrue(l))
      var_values.push_back(LitValue::True);
    else if(state.assignments_.IsAssignedFalse(l))
      var_values.push_back(LitValue::False);
    else
      var_values.push_back(LitValue::Unknown);
  }

  std::vector<std::vector<LitValue>> hist_values;
  for (auto h : hist) {
    std::vector<LitValue> h_v;
    for (BooleanLiteral l : h) {
      if (state.assignments_.IsAssignedTrue(l))
        h_v.push_back(LitValue::True);
      else if(state.assignments_.IsAssignedFalse(l))
        h_v.push_back(LitValue::False);
      else
        h_v.push_back(LitValue::Unknown);
    }
    hist_values.push_back(h_v);
  }
  int t_size = state.trail_.size();
  auto t = state.trail_;
  auto stop = state.propagator_clausal_.next_position_on_trail_to_propagate_it;
  state.propagator_clausal_.next_position_on_trail_to_propagate_it = state.trail_.begin();
  while (state.propagator_clausal_.next_position_on_trail_to_propagate_it != stop) {
    bool res =state.propagator_clausal_.PropagateOneLiteral(state);
    if (!res ){
      int shit = 1;
    }
  }
  int t_size2 =state.trail_.size();
  auto it = state.trail_.begin();
  auto i2 = state.trail_.end();

  std::vector<LitValue> var_values2;
  for (BooleanLiteral l : added_lit_hist_) {
    if (state.assignments_.IsAssignedTrue(l))
      var_values2.push_back(LitValue::True);
    else if(state.assignments_.IsAssignedFalse(l))
      var_values2.push_back(LitValue::False);
    else
      var_values2.push_back(LitValue::Unknown);
  }

  std::vector<std::vector<LitValue>> hist_values2;
  for (auto h : hist) {
    std::vector<LitValue> h_v;
    for (BooleanLiteral l : h) {
      if (state.assignments_.IsAssignedTrue(l))
        h_v.push_back(LitValue::True);
      else if(state.assignments_.IsAssignedFalse(l))
        h_v.push_back(LitValue::False);
      else
        h_v.push_back(LitValue::Unknown);
    }
    hist_values2.push_back(h_v);
  }

  int bet = 3;
}
bool IncrementalSequentialEncoder::UpdateMax(int max, SolverState &state) {
  assert(max < max_);
  if (sum_lits.empty()) {
    max_ = max;
  } else {
    for (int i = max; i < sum_lits.size(); ++i) {
      bool res = state.AddUnitClauseDuringSearch(~sum_lits[i]);
      if (!res)
        return res;
    }
  }
  return true;
}

} // namespace Pumpkin