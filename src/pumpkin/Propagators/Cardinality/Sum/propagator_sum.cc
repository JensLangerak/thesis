//
// Created by jens on 16-10-20.
//

#include "propagator_sum.h"
#include "../../../Engine/solver_state.h"
#include "../Encoders/incremental_sequential_encoder.h"
#include "../Encoders/totaliser_encoder.h"
#include "reason_sum_constraint.h"
#include "watch_list_sum.h"
#include <iostream>
namespace Pumpkin {

PropagatorSum::PropagatorSum(int64_t num_variables)
    : PropagatorGeneric(), sum_database_(num_variables), last_index_(0),
      last_propagated_(BooleanLiteral()) {}
bool PropagatorSum::PropagateLiteral(BooleanLiteral true_literal,
                                             SolverState &state) {
  assert(state.assignments_.IsAssignedTrue(true_literal));
  WatchListSum &watch_list_true = sum_database_.watch_list_true;
  std::vector<WatcherSumConstraint> &watchers_true =
      watch_list_true[true_literal];
  size_t end_position = 0;
  size_t current_index = 0;
  // Check if it has partly propagated the literal or not
  bool already_partly_done = false;
  if (!last_propagated_.IsUndefined() && last_propagated_ == true_literal) {
    current_index = last_index_;
    already_partly_done = true;
  } else {
    last_index_ = 0;
  }
  last_propagated_ = true_literal;
  assert(CheckCounts(state));
  // update all constraints that watch the literal
  for (; current_index < watchers_true.size(); ++current_index) {
    WatchedSumConstraint *constraint =
        watchers_true[current_index].constraint_;
    // if the encoding is added, no need to keep track of it anymore
    if (constraint->encoder_->EncodingAdded())
      continue;
    // only update the count the first time that the constraint is triggered.
    // //TODO not sure if the check is still needed
    if (last_index_ != current_index || (!already_partly_done)) {
      constraint->true_count_++;
      constraint->true_order_.push_back(true_literal);
    }
    already_partly_done = false;
    last_index_ = current_index;
    int true_count = 0;
    int false_count = 0;
    for (BooleanLiteral l : constraint->inputs_) {
      if (state.assignments_.IsAssignedTrue(l) &&
          state.assignments_.GetTrailPosition(l.Variable()) <=
              state.assignments_.GetTrailPosition(true_literal.Variable()))
        true_count++;
    }
//        assert(true_count == constraint->true_count_);
    constraint->true_count_ = true_count;
    true_count = constraint->true_count_;

    watchers_true[end_position] = watchers_true[current_index];
    ++end_position;
    if (true_count > constraint->outputs_.size())
      continue;
    bool success = state.EnqueuePropagatedLiteral(constraint->outputs_[true_count - 1], this, reinterpret_cast<uint64_t>(constraint));

    // conflict
    if (!success) {
      // restore remaining watchers
      for (size_t k = current_index + 1; k < watchers_true.size(); ++k) {
        watchers_true[end_position] = watchers_true[current_index];
        ++end_position;
      }
      watchers_true.resize(end_position);
      failure_constraint_ = constraint;
      //      return false;
      constraint->trigger_count_++;
      trigger_count_++;
      if (constraint->encoder_->AddEncodingDynamic()) {
        bool res = AddEncoding(state, constraint);
        //        state.FullReset();
        return res;
      } else {
        auto test1 = state.assignments_.IsAssigned(constraint->outputs_[true_count - 1]);
        auto test2 = state.assignments_.IsAssignedTrue(constraint->outputs_[true_count - 1]);
        return false;
      }
    }

  }

  watchers_true.resize(end_position);
  if (!next_position_on_trail_to_propagate_it.IsPastTrail())
    next_position_on_trail_to_propagate_it.Next();
  //  assert(CheckCounts(state));
  return true;
}
ReasonGeneric *PropagatorSum::ReasonFailure(SolverState &state) {
  return new ReasonSumConstraint(failure_constraint_, state);
}
ReasonGeneric *PropagatorSum::ReasonLiteralPropagation(
    BooleanLiteral propagated_literal, SolverState &state) {
  assert(state.assignments_.GetAssignmentPropagator(
             propagated_literal.Variable()) == this);
  uint64_t code =
      state.assignments_.GetAssignmentCode(propagated_literal.Variable());
  WatchedSumConstraint *propagating_constraint =
      reinterpret_cast<WatchedSumConstraint *>(code);
  // TODO check if encoding should be added (if called it is not yet added)
  return new ReasonSumConstraint(propagating_constraint,
                                         propagated_literal, state);
}
ExplanationGeneric *PropagatorSum::ExplainFailure(SolverState &state) {
  return failure_constraint_->ExplainFailure(state);
}
ExplanationGeneric *PropagatorSum::ExplainLiteralPropagation(
    BooleanLiteral propagated_literal, SolverState &state) {
  assert(state.assignments_.GetAssignmentPropagator(
             propagated_literal.Variable()) == this);
  uint64_t code =
      state.assignments_.GetAssignmentCode(propagated_literal.Variable());
  WatchedSumConstraint *propagating_constraint =
      reinterpret_cast<WatchedSumConstraint *>(code);
  // TODO check if encoding should be added (if called it is not yet added)
  return propagating_constraint->ExplainLiteralPropagation(propagated_literal,
                                                           state);
}

void PropagatorSum::Synchronise(SolverState &state) {

  // current literal is partly propagated, reduce the counts of the updated
  // constraints.
  if ((!last_propagated_.IsUndefined()) &&
      last_propagated_ == next_position_on_trail_to_propagate_it.GetData()) {
    BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
    if (!sum_database_.watch_list_true[l].empty()) {
      assert(last_index_ < sum_database_.watch_list_true[l].size());
      for (int i = 0; i <= last_index_; ++i) {
        sum_database_.watch_list_true[l][i].constraint_->true_count_--;

        sum_database_.watch_list_true[l][i].constraint_->true_order_.pop_back();
      }
      last_index_ = 0;
      // TODO roll back false
    }
  }

  // move back the iterator and updates the counts
  if (next_position_on_trail_to_propagate_it.IsPastTrail()) {
    while (next_position_on_trail_to_propagate_it != state.GetTrailEnd()) {
      next_position_on_trail_to_propagate_it.Previous();
      BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
      for (auto wc : sum_database_.watch_list_true[l]) {
        wc.constraint_->true_count_--;
        wc.constraint_->true_order_.pop_back();
      }
    }
    assert(next_position_on_trail_to_propagate_it == state.GetTrailEnd());
  }
  PropagatorGeneric::Synchronise(state);
  last_propagated_ = BooleanLiteral();
  last_index_ = 0;
}

bool PropagatorSum::PropagateOneLiteral(SolverState &state) {
  if (IsPropagationComplete(state) == false) {

    BooleanLiteral propagation_literal =
        *next_position_on_trail_to_propagate_it;
    bool success = PropagateLiteral(propagation_literal, state);
    return success;
  }
  return true; // no conflicts occurred during propagation
}

bool PropagatorSum::AddEncoding(
    SolverState &state, WatchedSumConstraint *constraint) {
  int clause_index =
      state.propagator_clausal_.clause_database_.permanent_clauses_.size();
  int unit_index =
      state.propagator_clausal_.clause_database_.unit_clauses_.size();
  int var_index = state.assignments_.GetNumberOfVariables() + 1;
  std::vector<std::vector<BooleanLiteral>> clauses =
      AddEncodingClauses(state, constraint);
//  std::priority_queue<PropagtionElement, std::vector<PropagtionElement>,
//                      std::greater<PropagtionElement>>
//      propagation_queue =
//          InitPropagationQueue(state, clauses, unit_index, clause_index);
//  UpdatePropagation(state, propagation_queue, var_index);
    state.FullReset();
  return true;
}
void PropagatorSum::ResetCounts() {
  for (auto c : sum_database_.permanent_constraints_) {
    c->true_count_ = 0;
    c->true_order_.clear();
    c->false_count_ = 0;
  }
}
bool PropagatorSum::ClausualPropagateLiteral(
    BooleanLiteral true_literal, SolverState &state,
    std::priority_queue<PropagtionElement, std::vector<PropagtionElement>,
                        std::greater<PropagtionElement>> &queue,
    int min_var) {
  assert(state.assignments_.IsAssignedTrue(true_literal));

  // effectively remove all watches from this true_literal
  // then go through the previous watches one by one and insert them as
  // indicated (some might be placed back in the watch list of this
  // true_literal) if a conflict takes place, put back the remaining clauses
  // into the watch list of this true_literal and report the conflict

  WatchList &watch_list =
      state.propagator_clausal_.clause_database_.watch_list_;
  std::vector<WatcherClause> &watches = watch_list[~true_literal];

  size_t end_position =
      0; // effectively, we are resizing the watch list to zero for this
         // literal, and in the loop we will add some of the old watches
  for (size_t current_index = 0; current_index < watches.size();
       current_index++) {
    assert(watches[current_index].clause_->literals_[0] == ~true_literal ||
           watches[current_index].clause_->literals_[1] ==
               ~true_literal); // one of the watched literals must be the
                               // opposite of the input true literal
    TwoWatchedClause *watched_clause = watches[current_index].clause_;

    // clause propagation starts here

    // place the considered literal at position 1 for simplicity
    if (watched_clause->literals_[0] == ~true_literal) {
      std::swap(watched_clause->literals_[0], watched_clause->literals_[1]);
    }

    auto test =watched_clause->literals_[0];
    // check the other watched literal to see if the clause is already satisfied
    if (state.assignments_.IsAssignedTrue(watched_clause->literals_[0]) ==
        true) {
      watches[current_index].cached_literal_ =
          watched_clause
              ->literals_[0]; // take the true literal as the new cache

      // Set watcher to most recent false literal
      int swap_index = 1;
      int decision_level = state.assignments_.GetAssignmentLevel(
          watched_clause->literals_[swap_index].Variable());
      int decision_level_true = state.assignments_.GetAssignmentLevel(
          watched_clause->literals_[0].Variable());
      if (decision_level > decision_level_true) {
        watches[end_position] = watches[current_index]; // keep the watch
        end_position++;
        continue;
      }

      for (int i = 2; i < watched_clause->literals_.Size();
           i++) // current_index = 2 since we are skipping the two watched
      // literals
      {
        if (!state.assignments_.IsAssigned(
                watched_clause->literals_[i].Variable())) {
          swap_index = i;
          decision_level = -1;
          break;
        }
        int decision_level_current = state.assignments_.GetAssignmentLevel(
            watched_clause->literals_[i].Variable());
        if (decision_level_current > decision_level_true) {
          decision_level = decision_level_current;
          swap_index = i;
          break;
        }
        if (decision_level_current > decision_level) {
          decision_level = decision_level_current;
          swap_index = i;
        }
      }
      std::vector<bool> assigned;
      std::vector<bool> assigned_value;
      std::vector<int> assigned_level;
      for (int i = 0; i < watched_clause->literals_.size_; ++i) {
        BooleanLiteral l = watched_clause->literals_[i];
        if (state.assignments_.IsAssigned(l.Variable())) {
          assigned.push_back(true);
          assigned_value.push_back(state.assignments_.IsAssignedTrue(l));
          assigned_level.push_back(
              state.assignments_.GetAssignmentLevel(l.Variable()));
        } else {
          assigned.push_back(false);
          assigned_value.push_back(false);
          assigned_level.push_back(-1);
        }
      }
      if (swap_index != 1) {
        std::swap(watched_clause->literals_[1],
                  watched_clause
                      ->literals_[swap_index]); // replace the watched literal
        watch_list.AddClauseToWatch(watched_clause->literals_[1],
                                    watched_clause);

      } else {
        watches[end_position] = watches[current_index]; // keep the watch
        end_position++;
      }
      if (decision_level < decision_level_true)
        queue.push(PropagtionElement(watched_clause->literals_[0], decision_level, &state.propagator_clausal_, reinterpret_cast<uint64_t>(watched_clause)));
      continue;
    }

    // look for another nonfalsified literal to replace one of the watched
    // literals
    bool found_new_watch = false;
    for (int i = 2; i < watched_clause->literals_.Size();
         i++) // current_index = 2 since we are skipping the two watched
              // literals
    {
      if (state.assignments_.IsAssignedFalse(watched_clause->literals_[i]) ==
          false) // not assigned false//not assigned false, can be either true
                 // or unassigned
      {
        // TODO: would it make sense to set the cached literal here if this new
        // literal will be set to true?
        std::swap(watched_clause->literals_[1],
                  watched_clause->literals_[i]); // replace the watched literal
        watch_list.AddClauseToWatch(
            watched_clause->literals_[1],
            watched_clause); // add the clause to the watch of the new watched
                             // literal
        found_new_watch = true;
        break; // no propagation is taking place, go to the next clause. Note
               // that since we did not increment end_position, we effectively
               // removed this clause from the watch of the input true_literal
        // TODO: it could be that literal[0] is false, and that the current
        // clause is actually unit with the new watched literal ->  does it make
        // sense to make the test now, or to leave it for another propagation
        // that will follow to discover this? I suppose it is not worth it since
        // conflicts are rare in the search
      }
    }

    if (found_new_watch) {
      continue;
    }

    // at this point, nonwatched literals and literal[1] are assigned false
    // therefore, this class is unit if literal_[0] is not assigned, or
    // satisfied/falsified if literal_[0] is true/false the latter happens if
    // literal[0] was enqueueud at some point but has not yet been propagated

    // Has become unit or false. Figure out at which level it could have
    // propagated a value and make sure that the watchers are correct

    BooleanLiteral prev_l0 = watched_clause->literals_[0];
    BooleanLiteral prev_l1 = watched_clause->literals_[1];
    assert(prev_l1 == ~true_literal);
    BooleanLiteral prev_other = prev_l0;

    bool is_assigned = state.assignments_.IsAssigned(prev_l0);
    // make sure that 0 is assigned at the highest level
    if (is_assigned && state.assignments_.GetAssignmentLevel(
                           watched_clause->literals_[0].Variable()) <
                           state.assignments_.GetAssignmentLevel(
                               watched_clause->literals_[1].Variable())) {
      std::swap(watched_clause->literals_[0], watched_clause->literals_[1]);
    }
    for (int i = 2; i < watched_clause->Size(); ++i) {
      if (state.assignments_.GetAssignmentLevel(
              watched_clause->literals_[1].Variable()) <
          state.assignments_.GetAssignmentLevel(
              watched_clause->literals_[i].Variable())) {
        std::swap(watched_clause->literals_[1], watched_clause->literals_[i]);
        if (is_assigned && state.assignments_.GetAssignmentLevel(
                               watched_clause->literals_[0].Variable()) <
                               state.assignments_.GetAssignmentLevel(
                                   watched_clause->literals_[1].Variable())) {
          std::swap(watched_clause->literals_[0], watched_clause->literals_[1]);
        }
      }
    }
    BooleanLiteral l0 = watched_clause->literals_[0];
    BooleanLiteral l1 = watched_clause->literals_[1];
    if (l0 == ~true_literal || l1 == ~true_literal) {
      watches[end_position] = watches[current_index];
      end_position++;
    }
    if (!(l0 == prev_other || l1 == prev_other)) {
      watch_list.RemoveClauseFromWatch(prev_other, watched_clause);
    }

    if (l0 != ~true_literal && l0 != prev_other)
      watch_list.AddClauseToWatch(l0, watched_clause);
    if (l1 != ~true_literal && l1 != prev_other)
      watch_list.AddClauseToWatch(l1, watched_clause);

    assert(!state.assignments_.IsAssigned(l0.Variable()) ||
           state.assignments_.GetAssignmentLevel(l0.Variable()) >=
               state.assignments_.GetAssignmentLevel(l1.Variable()));
    // try to enqueue the propagated assignment
    uint64_t code = reinterpret_cast<uint64_t>(
        watched_clause); // the code will simply be a pointer to the propagating
                         // clause
    int level = state.assignments_.GetAssignmentLevel(l1.Variable());
    queue.push(PropagtionElement(l0, level, &state.propagator_clausal_, code));

  }
  watches.resize(end_position);
  return true_literal.VariableIndex() >= min_var;
}

int PropagatorSum::PropagateLiterals(
    std::priority_queue<PropagtionElement, std::vector<PropagtionElement>,
                        std::greater<PropagtionElement>>
        queue,
    SolverState &state, int min_var_index) {
  int reset_level = state.GetCurrentDecisionLevel();
  while (!queue.empty()) {
    PropagtionElement p = queue.top();
    if (p.lit.VariableIndex() < min_var_index)
      return std::min(reset_level, p.level);
    if (reset_level <= p.level)
      return reset_level;
    queue.pop();
    if (p.propagator != NULL) {
      TwoWatchedClause *c = reinterpret_cast<TwoWatchedClause *>(p.code);
      int max_level = 0;
      for (int i = 0; i < c->literals_.Size(); ++i) {
        BooleanLiteral l = c->literals_[i];
        if (l == p.lit)
          continue;
        assert(state.assignments_.IsAssignedFalse(l));
        max_level = std::max(
            max_level, state.assignments_.GetAssignmentLevel(l.Variable()));
      }
      assert(max_level == p.level);
    } else {
      assert(p.level == 0);
    }

    if (p.level > state.GetCurrentDecisionLevel())
      assert(false);
    if (p.level == state.GetCurrentDecisionLevel()) {
      // no need to check the propagation, is handled by the normal propagators
      if (state.assignments_.IsAssignedFalse(p.lit)) {
        assert(
            state.assignments_.GetAssignmentLevel(p.lit.Variable()) <=
            p.level); // TODO should be equal when propagation works, can happen
                      // if an auxiliary var was selected as decision var?
        reset_level =
            std::min(state.assignments_.GetAssignmentLevel(p.lit.Variable()),
                     reset_level);
        // TODO set failure
      } else if (state.assignments_.IsAssignedTrue(p.lit)) {
        assert(state.assignments_.GetAssignmentLevel(p.lit.Variable()) <=
               p.level);
      } else {
        reset_level = std::min(reset_level,
                               p.level); // TODO figure out why this is needed
        state.EnqueuePropagatedLiteral(p.lit, p.propagator, p.code);
      }
      continue;
    }

    if (state.assignments_.IsAssigned(p.lit.Variable())) {
      // var can be assigned in an earlier level
      if (state.assignments_.IsAssignedTrue(p.lit)) {
        assert(state.assignments_.GetAssignmentLevel(p.lit.Variable()) <=
               p.level);
      } else {
        //        assert(false); // TODO what?
        reset_level = std::min(reset_level, p.level);
      }
    } else {
      //      if (p.lit.Variable().index_ >= min_var_index) {
      state.InsertPropagatedLiteral(p.lit, p.propagator, p.code, p.level);
      bool res = ClausualPropagateLiteral(p.lit, state, queue, min_var_index);
      if (!sum_database_.watch_list_true[p.lit].empty()) {
        reset_level = std::min(reset_level, p.level);
      }
      //      assert(sum_database_.watch_list_true[p.lit].empty());
      assert(queue.top().level >= p.level);
    }
  }
  // TODO update trail positions
  return reset_level;
}
void PropagatorSum::SetTrailIterator(
    TrailList<BooleanLiteral>::Iterator iterator) {
  // set the trail iterator and make sure the the counts stay correct.
  if (iterator.IsFirst()) {
    PropagatorGeneric::SetTrailIterator(iterator);
    ResetCounts();
  } else {

    if (!next_position_on_trail_to_propagate_it.IsPastTrail()) {
      BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
      for (auto wc : sum_database_.watch_list_true[l]) {
        wc.constraint_->true_count_--;
        wc.constraint_->true_order_.pop_back();
      }
    }
    while (next_position_on_trail_to_propagate_it != iterator) {
      next_position_on_trail_to_propagate_it.Previous();
      BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
      for (auto wc : sum_database_.watch_list_true[l]) {
        wc.constraint_->true_count_--;
        wc.constraint_->true_order_.pop_back();
      }
    }
  }
  assert(next_position_on_trail_to_propagate_it == iterator);
  last_propagated_ = BooleanLiteral();
  last_index_ = 0;
}
bool PropagatorSum::CheckCounts(SolverState &state) {
  return true;
  TrailList<BooleanLiteral>::Iterator counter_it = state.GetTrailBegin();
  for (auto c : sum_database_.permanent_constraints_) {
    c->true_count_debug_ = 0;
    //    c->true_log_debug.clear();
  }
  while (counter_it != next_position_on_trail_to_propagate_it) {
    BooleanLiteral l = counter_it.GetData();
    for (auto c : sum_database_.watch_list_true[l]) {
      c.constraint_->true_count_debug_++;
      //      c.constraint_->true_log_debug.emplace_back(c.constraint_->true_count_debug_,
      //      l);
    }
    counter_it.Next();
  }
  int miscount = 0;
  int l = 0;
  int g = 0;
  for (auto c : sum_database_.permanent_constraints_) {
    //    assert(c->true_count_debug_ == c->true_count_);
    if (c->true_count_debug_ != c->true_count_) {
      miscount++;
      if (c->true_count_debug_ > c->true_count_)
        ++g;
      else
        ++l;
    }
  }

  assert(miscount == 0);
  return true;
}
bool PropagatorSum::PropagateIncremental(
    SolverState &state, WatchedSumConstraint *constraint) {

  int clause_index =
      state.propagator_clausal_.clause_database_.permanent_clauses_.size();
  int unit_index =
      state.propagator_clausal_.clause_database_.unit_clauses_.size();
  int var_index = state.assignments_.GetNumberOfVariables() + 1;

  std::vector<BooleanLiteral> reason;
  std::vector<BooleanLiteral> propagate;
  for (int i = 0; i < constraint->inputs_.size(); ++i) {
    BooleanLiteral l = constraint->inputs_[i];
    if ((!state.assignments_.IsAssigned(
            l.Variable()))) // && (!constraint->encoder_->IsAdded(l)))
      propagate.push_back(l);
    else if (state.assignments_.IsAssignedTrue(l))
      reason.push_back(l);
  }

  if (propagate.empty()) {
    //    state.FullReset();
    return false;
  }
  std::vector<std::vector<BooleanLiteral>> clauses =
      AddEncodingClauses(state, constraint);
  bool clause_empty = clauses.empty();
  if (clauses.empty()) {
    assert(!constraint->encoder_->IsAdded(propagate[0]));
    std::vector<std::vector<BooleanLiteral>> propagate_clauses =
        //      constraint->encoder_->Encode(state, {propagate[0]});
        constraint->encoder_->Propagate(state, reason, propagate);
    for (auto c : propagate_clauses)
      clauses.push_back(c);
  } else {
//    state.FullReset();
//    return true;
  }


//  std::cout << "--------------  " << constraint->true_count_<< std::endl;
  ((IncrementalSequentialEncoder *)constraint->encoder_)->PrintState(state);

  std::priority_queue<PropagtionElement, std::vector<PropagtionElement>,
                      std::greater<PropagtionElement>>
      propagation_queue =
          InitPropagationQueue(state, clauses, unit_index, clause_index);
  UpdatePropagation(state, propagation_queue, var_index);

  ((IncrementalSequentialEncoder *)constraint->encoder_)->PrintState(state);
    return true;
}

std::vector<std::vector<BooleanLiteral>>
PropagatorSum::AddEncodingClauses(
    SolverState &state, WatchedSumConstraint *constraint) {
  std::vector<std::vector<BooleanLiteral>> clauses;
  if (constraint->encoder_->SupportsIncremental()) {
    std::vector<BooleanLiteral> cause;
    int level_count = 0;
    for (BooleanLiteral l : constraint->inputs_) {
      if (state.assignments_.IsAssignedTrue(l)) {
        cause.push_back(l);
        if (state.assignments_.GetAssignmentLevel(l.Variable()) ==
            state.GetCurrentDecisionLevel())
          level_count++;
      }
    }

    clauses = constraint->encoder_->Encode(state, cause);
  } else {
    assert(constraint->encoder_->EncodingAdded() == false);
    clauses = constraint->encoder_->Encode(state);
  }
  return clauses;
}
std::priority_queue<PropagatorSum::PropagtionElement,
                    std::vector<PropagatorSum::PropagtionElement>,
                    std::greater<PropagatorSum::PropagtionElement>>
PropagatorSum::InitPropagationQueue(
    SolverState &state, std::vector<std::vector<BooleanLiteral>> clauses,
    int unit_start_index, int clause_start_index) {
  std::priority_queue<PropagtionElement, std::vector<PropagtionElement>,
                      std::greater<PropagtionElement>>
      propagation_queue;
  for (int i = unit_start_index;
       i < state.propagator_clausal_.clause_database_.unit_clauses_.size();
       ++i) {
    // add unit classes to the queue.
    BooleanLiteral l =
        state.propagator_clausal_.clause_database_.unit_clauses_[i];
      propagation_queue.push(PropagtionElement(l, 0, NULL, NULL));
  }
  for (int i = clause_start_index;
       i < state.propagator_clausal_.clause_database_.permanent_clauses_.size();
       ++i) {
    TwoWatchedClause *c =
        state.propagator_clausal_.clause_database_.permanent_clauses_[i];
    BooleanLiteral l1 = c->literals_[0];
    BooleanLiteral l2 = c->literals_[1];
    // make sure that l1 is unassigned or true, or both are false.
    if (state.assignments_.IsAssignedFalse(l1)) {
      l1 = c->literals_[1];
      l2 = c->literals_[0];
    }
    assert((!state.assignments_.IsAssigned(l1)) ||
           (state.assignments_.IsAssignedTrue(l1)) ||
           (state.assignments_.IsAssignedFalse(l2)));
    auto test1 = state.assignments_.IsAssignedTrue(l1.Variable());
    auto test2 = state.assignments_.IsAssigned(l1.Variable());
    auto test3 = state.assignments_.IsAssignedTrue(l2.Variable());
    auto test4 = state.assignments_.IsAssigned(l2.Variable());
    if ((!state.assignments_.IsAssignedTrue(l1)) &&
        state.assignments_.IsAssignedFalse(l2)) {

      int max_level = state.assignments_.GetAssignmentLevel(l2.Variable());
      propagation_queue.push(PropagtionElement(l1, max_level,
                                               &state.propagator_clausal_,
                                               reinterpret_cast<uint64_t>(c)));
    } else if (state.assignments_.IsAssignedFalse(l2) && state.assignments_.GetAssignmentLevel(l2.Variable()) < state.assignments_.GetAssignmentLevel(l1.Variable())) {
      int index = l2 == c->literals_[0] ? 0 : 1;
      for (int i = 2; i < c->literals_.size_; ++i) {
        if (state.assignments_.GetAssignmentLevel(c->literals_[index].Variable()) < state.assignments_.GetAssignmentLevel(c->literals_[i].Variable())) {
          index = i;
        }
      }

      assert (index <= 1);
      int max_level = state.assignments_.GetAssignmentLevel(l2.Variable());
      propagation_queue.push(PropagtionElement(l1, max_level,
                                               &state.propagator_clausal_,
                                               reinterpret_cast<uint64_t>(c)));
    }
  }

  return propagation_queue;
}
void PropagatorSum::UpdatePropagation(
    SolverState &state,
    std::priority_queue<PropagtionElement, std::vector<PropagtionElement>,
                        std::greater<PropagtionElement>>
        propagation_queue,
    int min_var_index) {
//  state.FullReset(); //Sometimes faster (especially for incremental TODO configure and test)
//  return;


  int decision_level = state.GetCurrentDecisionLevel();
  // propagate the newly added clauses and vars
  int backtrack_level =
      PropagateLiterals(propagation_queue, state, min_var_index);
  RepairTrailPositions(state);
  // TODO handle backtrack more uniform (either return the level or return void
  // and do the backtrack in the method
  backtrack_level = std::min(state.GetCurrentDecisionLevel(),
                             backtrack_level); // TODO find bug
//  std::cout << "b: "<< backtrack_level << std::endl;
//  backtrack_level = 0;
   if (backtrack_level <= 0)
    state.FullReset();
  else {
    if (backtrack_level < state.GetCurrentDecisionLevel())
      state.Backtrack(backtrack_level);
    else if (decision_level != backtrack_level)
      Synchronise(state); // TODO move to reset propagators


    // backtrack backtracks to the end of the decision level.
    // However the propagation might have inserted propagation values to that
    // level that should be considered. Currently we do not know which values
    // were inserted, thus reset to the beginning of the level. If there was no
    // backtrack than the newly added propagation values are added to the end,
    // thus no reset needed.
      state.ResetPropagatorsToLevel();


  }
  RepairTrailPositions(state);


}
void PropagatorSum::RepairTrailPositions(SolverState &state) {
  auto it = state.GetTrailBegin();
  int position = 0;
  int decisionlevel = 0;
  while (!it.IsLast()) {
    state.assignments_.info_[it.GetData().VariableIndex()].position_on_trail =
        position;
    if (state.assignments_.GetAssignmentLevel(it.GetData().Variable()) ==
        decisionlevel + 1)
      decisionlevel++;
    auto test = state.assignments_.info_[it.GetData().VariableIndex()];
    assert(state.assignments_.GetAssignmentLevel(it.GetData().Variable()) ==
           decisionlevel);
    ++position;
    it.Next();
  }
}
} // namespace Pumpkin
