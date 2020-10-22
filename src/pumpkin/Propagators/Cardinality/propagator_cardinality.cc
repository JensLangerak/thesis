//
// Created by jens on 16-10-20.
//

#include "propagator_cardinality.h"
#include "../../Engine/solver_state.h"
#include "reason_cardinality_constraint.h"
#include "totaliser_encoder.h"
#include "watch_list_cardinality.h"
namespace Pumpkin {

PropagatorCardinality::PropagatorCardinality(int64_t num_variables)
    : PropagatorGeneric(), cardinality_database_(num_variables), last_index_(0),
      last_propagated_(BooleanLiteral()) {}
bool PropagatorCardinality::PropagateLiteral(BooleanLiteral true_literal,
                                             SolverState &state) {
  assert(state.assignments_.IsAssignedTrue(true_literal));
  WatchListCardinality &watch_list_true = cardinality_database_.watch_list_true;
  std::vector<WatcherCardinalityConstraint> &watchers_true =
      watch_list_true[true_literal];
  size_t end_position = 0;
  size_t current_index = 0;
  bool already_partly_done = false;
  if (!last_propagated_.IsUndefined() && last_propagated_ == true_literal) {
    current_index = last_index_;
    already_partly_done = true;
  } else {
    last_index_ = 0;
  }
  last_propagated_ = true_literal;
  for (; current_index < watchers_true.size(); ++current_index) {
    WatchedCardinalityConstraint *constraint =
        watchers_true[current_index].constraint_;
    if (last_index_ != current_index || (!already_partly_done)) {
      constraint->true_count_++;
      constraint->true_log.emplace_back(constraint->true_count_, true_literal);
    }
    already_partly_done = false;
    last_index_ = current_index;
    int true_count = 0;
    int false_count = 0;
    int unassinged_count = 0;
    for (BooleanLiteral l : constraint->literals_) {
      if (!state.assignments_.IsAssigned(l) ||
          // TODO don;t think that this still works (is debug code anyway)
          state.assignments_.GetTrailPosition(l.Variable()) >
              state.assignments_.GetTrailPosition(true_literal.Variable())) {
        ++unassinged_count;
      } else if (state.assignments_.IsAssignedTrue(l)) {
        ++true_count;
      } else {
        ++false_count;
      }
    }
    // TODO handle with propagation
    //    constraint->true_count_ = true_count;
    assert(constraint->true_count_ == true_count);
    constraint->false_count_ = false_count;
    //    int true_count = constraint->true_count_;
    //    int false_count = constraint->false_count_;

    watchers_true[end_position] = watchers_true[current_index];
    ++end_position;
    if (true_count > constraint->max_ ||
        false_count > constraint->literals_.size() - constraint->min_) {
      // restore remaining watchers
      for (size_t k = current_index + 1; k < watchers_true.size(); ++k) {
        watchers_true[end_position] = watchers_true[current_index];
        ++end_position;
      }
      watchers_true.resize(end_position);
      failure_constraint_ = constraint;
      //      return false;
      AddEncoding(state, constraint);
      return true;
    }
    if (true_count == constraint->max_ ||
        false_count == constraint->literals_.size() - constraint->min_) {
      for (BooleanLiteral l : constraint->literals_) {

        if (!state.assignments_.IsAssigned(l)) {
          AddEncoding(state, constraint);
          return true;
          uint64_t code = reinterpret_cast<uint64_t>(
              constraint); // the code will simply be a pointer to the
                           // propagating clause
          if (true_count == constraint->max_)
            state.EnqueuePropagatedLiteral(~l, this, code);
          else
            state.EnqueuePropagatedLiteral(l, this, code);
          return true;
        }
      }
    }
  }

  watchers_true.resize(end_position);
  //  next_position_on_trail_to_propagate_++;
  next_position_on_trail_to_propagate_it.Next();
  return true;
}
ReasonGeneric *PropagatorCardinality::ReasonFailure(SolverState &state) {
  return new ReasonCardinalityConstraint(failure_constraint_, state);
}
ReasonGeneric *PropagatorCardinality::ReasonLiteralPropagation(
    BooleanLiteral propagated_literal, SolverState &state) {
  assert(state.assignments_.GetAssignmentPropagator(
             propagated_literal.Variable()) == this);
  uint64_t code =
      state.assignments_.GetAssignmentCode(propagated_literal.Variable());
  WatchedCardinalityConstraint *propagating_constraint =
      reinterpret_cast<WatchedCardinalityConstraint *>(code);
  return new ReasonCardinalityConstraint(propagating_constraint,
                                         propagated_literal, state);
}
ExplanationGeneric *PropagatorCardinality::ExplainFailure(SolverState &state) {
  return failure_constraint_->ExplainFailure(state);
}
ExplanationGeneric *PropagatorCardinality::ExplainLiteralPropagation(
    BooleanLiteral propagated_literal, SolverState &state) {
  assert(state.assignments_.GetAssignmentPropagator(
             propagated_literal.Variable()) == this);
  uint64_t code =
      state.assignments_.GetAssignmentCode(propagated_literal.Variable());
  WatchedCardinalityConstraint *propagating_constraint =
      reinterpret_cast<WatchedCardinalityConstraint *>(code);
  return propagating_constraint->ExplainLiteralPropagation(propagated_literal,
                                                           state);
}

void PropagatorCardinality::Synchronise(SolverState &state) {

  // TODO do this before the trail is rolled back
  if ((!last_propagated_.IsUndefined()) &&
      last_propagated_ == next_position_on_trail_to_propagate_it.GetData()) {
    BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
    if (!cardinality_database_.watch_list_true[l].empty()) {
      assert(last_index_ < cardinality_database_.watch_list_true[l].size());
      for (int i = 0; i <= last_index_; ++i) {

        assert(cardinality_database_.watch_list_true[l][i]
                   .constraint_->true_log.back()
                   .count == cardinality_database_.watch_list_true[l][i]
                                 .constraint_->true_count_);
        assert(cardinality_database_.watch_list_true[l][i]
                   .constraint_->true_log.back()
                   .lit == l);
        cardinality_database_.watch_list_true[l][i].constraint_->true_count_--;
        cardinality_database_.watch_list_true[l][i]
            .constraint_->true_log.pop_back();
        assert(cardinality_database_.watch_list_true[l][i]
                       .constraint_->true_log.empty() &&
                   cardinality_database_.watch_list_true[l][i]
                           .constraint_->true_count_ == 0 ||
               cardinality_database_.watch_list_true[l][i]
                       .constraint_->true_log.back()
                       .count == cardinality_database_.watch_list_true[l][i]
                                     .constraint_->true_count_);
      }
      last_index_ = 0;
      // TODO roll back false
    }
  }
  //  if (next_position_on_trail_to_propagate_ >
  //  state.GetNumberOfAssignedVariables()) {
  if (next_position_on_trail_to_propagate_it.IsPastTrail()) {
    while (next_position_on_trail_to_propagate_it != state.GetTrailEnd()) {
      next_position_on_trail_to_propagate_it.Previous();
      //      --next_position_on_trail_to_propagate_;
      BooleanLiteral l = next_position_on_trail_to_propagate_it.GetData();
      for (auto wc : cardinality_database_.watch_list_true[l]) {
        assert(wc.constraint_->true_log.back().count ==
               wc.constraint_->true_count_);
        assert(wc.constraint_->true_log.back().lit == l);
        wc.constraint_->true_count_--;
        wc.constraint_->true_log.pop_back();
        assert(wc.constraint_->true_log.empty() &&
                   wc.constraint_->true_count_ == 0 ||
               wc.constraint_->true_log.back().count ==
                   wc.constraint_->true_count_);
      }
      for (auto wc : cardinality_database_.watch_list_false[~l]) {
        //      wc.constraint_->false_count_--;
      }
    }
    //    assert(next_position_on_trail_to_propagate_ ==
    //    state.GetNumberOfAssignedVariables());
    assert(next_position_on_trail_to_propagate_it == state.GetTrailEnd());
  } else {
    //    assert(next_position_on_trail_to_propagate_ <
    //    state.GetNumberOfAssignedVariables());
  }
  PropagatorGeneric::Synchronise(state);
  last_propagated_ = BooleanLiteral();
  last_index_ = 0;
}

bool PropagatorCardinality::PropagateOneLiteral(SolverState &state) {
  if (IsPropagationComplete(state) == false) {

    //    BooleanLiteral propagation_literal =
    //    state.GetLiteralFromTrailAtPosition(
    //        next_position_on_trail_to_propagate_);
    BooleanLiteral propagation_literal2 =
        *next_position_on_trail_to_propagate_it;
    //        assert(propagation_literal == propagation_literal2);
    bool success = PropagateLiteral(propagation_literal2, state);
    if (success == false) {
      return false;
    }
  }
  return true; // no conflicts occurred during propagation
}
void PropagatorCardinality::AddEncoding(
    SolverState &state, WatchedCardinalityConstraint *constraint) {
  assert(constraint->encoding_added_ == false);
  int clause_index =
      state.propagator_clausal_.clause_database_.permanent_clauses_.size();
  int unit_index =
      state.propagator_clausal_.clause_database_.unit_clauses_.size();
  int var_index = state.assignments_.GetNumberOfVariables() + 1;
  std::vector<std::vector<BooleanLiteral>> clauses = TotaliserEncoder::Encode(
      state, constraint->literals_, constraint->min_, constraint->max_);
  constraint->encoding_added_ = true;
  std::priority_queue<PropagtionElement, std::vector<PropagtionElement>,
                      std::greater<PropagtionElement>>
      prop_queue;
  for (int i = unit_index;
       i < state.propagator_clausal_.clause_database_.unit_clauses_.size();
       ++i) {
    BooleanLiteral l =
        state.propagator_clausal_.clause_database_.unit_clauses_[i];
    if (state.assignments_.IsAssigned(l)) {
    } else {
      prop_queue.push(PropagtionElement(l, 0, NULL, NULL));
    }
  }

  for (int i = clause_index;
       i < state.propagator_clausal_.clause_database_.permanent_clauses_.size();
       ++i) {
    TwoWatchedClause *c =
        state.propagator_clausal_.clause_database_.permanent_clauses_[i];
    BooleanLiteral l1 = c->literals_[0];
    BooleanLiteral l2 = c->literals_[1];
    if (state.assignments_.IsAssignedFalse(l1)) {
      l1 = c->literals_[1];
      l2 = c->literals_[0];
    }
    assert((!state.assignments_.IsAssigned(l1)) ||
           (state.assignments_.IsAssignedTrue(l1)) ||
           (state.assignments_.IsAssignedFalse(l2)));
    if ((!state.assignments_.IsAssignedTrue(l1)) &&
        state.assignments_.IsAssignedFalse(l2)) {
//      std::vector<int> test;
//      for (int k = 0; k < c->literals_.size_; ++k) {
//        BooleanLiteral l = c->literals_[k];
//        if (state.assignments_.IsAssignedTrue(l))
//          test.push_back(1);
//        else if (state.assignments_.IsAssignedFalse(l))
//          test.push_back(2);
//        else
//          test.push_back(0);
//      }
//      for (int j = 2; j < c->literals_.size_; j++) {
//        assert(state.assignments_.IsAssignedFalse(c->literals_[j]));
//      }

      int max_level = state.assignments_.GetAssignmentLevel(l2.Variable());
//      for (int j = 2; j < c->literals_.size_; j++) {
//        BooleanLiteral l = c->literals_[j];
//
//        max_level = std::max(
//            max_level, state.assignments_.GetAssignmentLevel(l.Variable()));
//      }
      assert(max_level == state.assignments_.GetAssignmentLevel(l2.Variable()));
      prop_queue.push(PropagtionElement(l1, max_level,
                                        &state.propagator_clausal_,
                                        reinterpret_cast<uint64_t>(c)));
    }
  }

  int backtrack_level = PropagateLiterals(prop_queue, state, var_index);
  if (backtrack_level == 0)
    state.FullReset();
  else
  state.Backtrack(backtrack_level - 1);
}
void PropagatorCardinality::ResetCounts() {
  for (auto c : cardinality_database_.permanent_constraints_) {
    c->true_count_ = 0;
    c->false_count_ = 0;
  }
}
int PropagatorCardinality::PropagateLiteral2(
    BooleanLiteral true_literal, SolverState &state,
    std::priority_queue<PropagtionElement, std::vector<PropagtionElement>,
                        std::greater<PropagtionElement>> &queue, int min_var) {
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

    // inspect if the cached true_literal is already set to true
    // if so, no need to go further in the memory to check the clause
    // often this literal will be true in practice so it is a good heuristic to
    // check
    if (state.assignments_.IsAssignedTrue(
            watches[current_index].cached_literal_)) {
      watches[end_position] = watches[current_index]; // keep the watch
      end_position++;
      continue;
    }

    TwoWatchedClause *watched_clause = watches[current_index].clause_;

    // clause propagation starts here

    // place the considered literal at position 1 for simplicity
    if (watched_clause->literals_[0] == ~true_literal) {
      std::swap(watched_clause->literals_[0], watched_clause->literals_[1]);
    }

    // check the other watched literal to see if the clause is already satisfied
    if (state.assignments_.IsAssignedTrue(watched_clause->literals_[0]) ==
        true) {
      watches[current_index].cached_literal_ =
          watched_clause
              ->literals_[0]; // take the true literal as the new cache
      watches[end_position] = watches[current_index]; // keep the watch
      end_position++;
      continue; // the clause is satisfied, no propagation can take place, go to
                // the next clause
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

    assert(!state.assignments_.IsAssigned(l0.Variable()) ||   state.assignments_.GetAssignmentLevel(l0.Variable()) >= state.assignments_.GetAssignmentLevel(l1.Variable()));
    // try to enqueue the propagated assignment
    uint64_t code = reinterpret_cast<uint64_t>(
        watched_clause); // the code will simply be a pointer to the propagating
                         // clause
    int level = state.assignments_.GetAssignmentLevel(l1.Variable());
    if (l0.Variable().index_ >= min_var) {
      queue.push(
          PropagtionElement(l0, level, &state.propagator_clausal_, code));
    } else {
      while (state.GetCurrentDecisionLevel() > level) {
        state.BacktrackOneLevel();
      }
    }
  }
  watches.resize(end_position);
  return -1; // no conflicts detected
}

int PropagatorCardinality::PropagateLiterals(
    std::priority_queue<PropagtionElement, std::vector<PropagtionElement>,
                        std::greater<PropagtionElement>>
        queue,
    SolverState &state, int min_var_index) {
  int reset_level = state.GetCurrentDecisionLevel();
  while (!queue.empty()) {
    PropagtionElement p = queue.top();
    queue.pop();
    if (p.level > state.GetCurrentDecisionLevel())
      return reset_level;
    if (p.lit.Variable().index_ < min_var_index) {
      reset_level = p.level;
      continue;
    }
    if (state.assignments_.IsAssigned(p.lit.Variable())) {
      if (state.assignments_.IsAssignedTrue(p.lit)) {
        assert(state.assignments_.GetAssignmentLevel(p.lit.Variable()) <=
               p.level);
      } else {
        reset_level = p.level;
      }
    } else {
      if (p.lit.Variable().index_ >= min_var_index) {
        state.InsertPropagatedLiteral(p.lit, p.propagator, p.code, p.level);
        int l = PropagateLiteral2(p.lit, state, queue, min_var_index);
        if (l > -1)
          reset_level = std::min(reset_level, l);
        assert(queue.top().level >= p.level);
      } else {
        state.Backtrack(p.level);
        while (state.GetCurrentDecisionLevel() > p.level) {
          state.BacktrackOneLevel();
        }
        reset_level = std::min(reset_level, p.level);
      }
    }
  }
  return reset_level;
}

} // namespace Pumpkin
