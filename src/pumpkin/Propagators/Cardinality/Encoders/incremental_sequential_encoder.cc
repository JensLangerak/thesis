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
    for (BooleanLiteral l : variables_) {
      state.propagator_cardinality_.cardinality_database_.watch_list_sum_true.Add(l, cardinality_constraint_);
    }
  }
  if (last_hist_size == -1) {
    last_hist_size = hist.size();
    last_add_clauses_index = state.propagator_clausal_.clause_database_.permanent_clauses_.size();
  }
  state.propagator_cardinality_.cardinality_database_.watch_list_sum_true.Remove(l, cardinality_constraint_);


  //TODO debug code; remove
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


  //  if (!added_lits_.empty()) {
  assert(previous_added_lits_.size() == current_added_lits.size());

  for (int i = 0; i < max_; ++i) {
    added_clauses.push_back({~current_added_lits[i], previous_added_lits_[i]});
  }

  added_clauses.push_back({previous_added_lits_[0], ~l});
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
//  return;
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
void IncrementalSequentialEncoder::RepairReasons(SolverState &state) {
  if (last_hist_size == -1)
    return;
  int watched_index = last_hist_size - 1;
  assert(hist.size() > last_hist_size);
  std::vector<BooleanLiteral> &watched_lits = hist[watched_index];
  std::vector<BooleanLiteral> &connected_lits = hist[last_hist_size];
  BooleanLiteral input_literal = added_lits_[watched_index];
  for (int i = 0; i < watched_lits.size(); ++i) {
    BooleanLiteral l = watched_lits[i];
    if (state.assignments_.IsAssigned(l.Variable()) &&
        state.assignments_.GetAssignmentPropagator(l.Variable()) == &state.propagator_sum_) {
      assert(state.assignments_.IsAssignedTrue(l.Variable()));
      bool previous_i =
          state.assignments_.IsAssignedTrue(connected_lits[i]);
      TwoWatchedClause *c;
      if (previous_i) {
        c = state.propagator_clausal_.clause_database_.permanent_clauses_[last_add_clauses_index + i];
        assert(c->literals_.size_ == 2);
        assert(c->literals_[0] == l || c->literals_[1] == l);
        assert(c->literals_[0] == ~connected_lits[i] ||
            c->literals_[1] == ~connected_lits[i]);
        assert(state.assignments_.GetTrailPosition(connected_lits[i].Variable()) < state.assignments_.GetTrailPosition(l.Variable()));
      } else {

        assert(state.assignments_.IsAssignedTrue(input_literal));

        c = state.propagator_clausal_.clause_database_
            .permanent_clauses_[max_ + i + last_add_clauses_index];
        if (i > 0) {
          BooleanLiteral prev_state_lit = connected_lits[i - 1];
          assert(
              state.assignments_.IsAssignedTrue(prev_state_lit));

          assert(c->literals_.size_ == 3);
          assert(c->literals_[0] == ~prev_state_lit || c->literals_[1] == ~prev_state_lit ||
              c->literals_[2] == ~prev_state_lit);
          assert(c->literals_[0] == l || c->literals_[1] == l ||
              c->literals_[2] == l);
          assert(c->literals_[0] == ~input_literal ||
              c->literals_[1] == ~input_literal ||
              c->literals_[2] == ~input_literal);

          assert(state.assignments_.GetTrailPosition(prev_state_lit.Variable()) < state.assignments_.GetTrailPosition(l.Variable()));
          assert(state.assignments_.GetTrailPosition(input_literal.Variable()) < state.assignments_.GetTrailPosition(l.Variable()));
        } else {

          assert(c->literals_.size_ == 2);
          assert(c->literals_[0] == l || c->literals_[1] == l);
          assert(c->literals_[0] == ~input_literal ||
              c->literals_[1] == ~input_literal);

          assert(state.assignments_.GetTrailPosition(input_literal.Variable()) < state.assignments_.GetTrailPosition(l.Variable()));
        }
      }
      int level = 0;
      for (int i = 0; i < c->literals_.size_; i++) {
        if (c->literals_[i] == l)
          continue;
        assert(state.assignments_.IsAssignedFalse(c->literals_[i]));
        level = std::max(
            state.assignments_.GetAssignmentLevel(c->literals_[i].Variable()),
            level);
      }
      assert(level = state.assignments_.info_[l.VariableIndex()].decision_level);

//      state.UndoAssignment(l);
      //TODO unasign?
      state.assignments_.info_[l.VariableIndex()].responsible_propagator =
          &state.propagator_clausal_;
      state.assignments_.info_[l.VariableIndex()].code =
          reinterpret_cast<uint64_t>(c);
    }
  }
  last_hist_size = -1;

}

} // namespace Pumpkin