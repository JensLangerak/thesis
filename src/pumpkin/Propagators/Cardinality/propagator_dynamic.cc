//
// Created by jens on 28-11-20.
//

#include "propagator_dynamic.h"
#include "../../Engine/solver_state.h"
namespace Pumpkin {

template <class T>
bool PropagatorDynamic<T>::AddEncoding(SolverState &state,
                                    T *constraint) {
  int clause_index =
      state.propagator_clausal_.clause_database_.permanent_clauses_.size();
  int unit_index =
      state.propagator_clausal_.clause_database_.unit_clauses_.size();
  int var_index = state.assignments_.GetNumberOfVariables() + 1; // 0 is unused, therefore + 1
  AddEncodingClauses(state, constraint);
  PropagateAddedClauses(state, unit_index, clause_index, var_index);
  return true;
}

template <class T>
bool PropagatorDynamic<T>::ClausualPropagateLiteral(
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

    // check the other watched literal to see if the clause is already satisfied
    if (state.assignments_.IsAssignedTrue(watched_clause->literals_[0]) ==
        true) {
      watches[current_index].cached_literal_ =
          watched_clause
              ->literals_[0]; // take the true literal as the new cache

      // Set watcher to most recent false literal, or to a literal that is set after the true literal.
      // One watcher watches the true literal, the other should watch a literal that is set after the true literal or that is not yet set to false.
      int swap_index = 1;
      int decision_level = state.assignments_.GetAssignmentLevel(
          watched_clause->literals_[swap_index].Variable());
      int decision_level_true = state.assignments_.GetAssignmentLevel(
          watched_clause->literals_[0].Variable());
      if (decision_level > decision_level_true) { // current watched is set after the true_literal, so we are done.
        watches[end_position] = watches[current_index]; // keep the watch
        end_position++;
        continue;
      }

      for (int i = 2; i < watched_clause->literals_.Size();
           i++) // current_index = 2 since we are skipping the two watched
      // literals
      {
        // found a literal that is not yet set or that is true, so we use that one.
        if (!state.assignments_.IsAssignedFalse(
                watched_clause->literals_[i])) {
          swap_index = i;
          decision_level = -1;
          break;
        }
        // check if literal is set later that the current latest literal.
        int decision_level_current = state.assignments_.GetAssignmentLevel(
            watched_clause->literals_[i].Variable());
        if (decision_level_current > decision_level_true) {
          decision_level = decision_level_current;
          swap_index = i;
          break;
        }

        // found a literal that is set after the true literal
        if (decision_level_current > decision_level) {
          decision_level = decision_level_current;
          swap_index = i;
        }
      }

      // swap the literal and set the watch
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

      // true literal should have been propagated before
      if (decision_level < decision_level_true && decision_level > -1)
        queue.push(
            PropagtionElement(watched_clause->literals_[0], decision_level,
                              &state.propagator_clausal_,
                              reinterpret_cast<uint64_t>(watched_clause)));
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
          false) //not assigned false, can be either true or unassigned
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
    // update watch lists
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

template <class T>
int PropagatorDynamic<T>::PropagateLiterals(
    std::priority_queue<PropagtionElement, std::vector<PropagtionElement>,
                        std::greater<PropagtionElement>> &
        queue,
    SolverState &state, int min_var_index) {
  int reset_level = state.GetCurrentDecisionLevel();
  while (!queue.empty()) {
    PropagtionElement p = queue.top();
    queue.pop();
    // handle the rest with backtrack
    if (p.lit.VariableIndex() < min_var_index)
      return std::min(reset_level, p.level);
    if (reset_level <= p.level)
      return reset_level;
    if (p.propagator != NULL) {
      TwoWatchedClause *c = reinterpret_cast<TwoWatchedClause *>(p.code);
      int max_level = 0;
      // TODO remove debug code
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
        //        assert(false); // TODO can this still happen?
        reset_level = std::min(reset_level, p.level);
      }
    } else {
      //      if (p.lit.Variable().index_ >= min_var_index) {
      state.InsertPropagatedLiteral(p.lit, p.propagator, p.code, p.level);
      bool res = ClausualPropagateLiteral(p.lit, state, queue, min_var_index);
      // TODO
      //      if (!cardinality_database_.watch_list_true[p.lit].empty()) {
      //        reset_level = std::min(reset_level, p.level);
      //      }
      //      assert(cardinality_database_.watch_list_true[p.lit].empty());
      assert(queue.top().level >= p.level);
    }
  }
  // TODO update trail positions
  return reset_level;
}

template <class T>
std::vector<std::vector<BooleanLiteral>>
PropagatorDynamic<T>::AddEncodingClauses(SolverState &state,
                                      T *constraint) {
  std::vector<std::vector<BooleanLiteral>> clauses;
  IEncoder * encoder = GetEncoder(constraint);
  if (encoder->SupportsIncremental()) {
    std::vector<BooleanLiteral> cause = GetEncodingCause(state, constraint);
    clauses = encoder->Encode(state, cause);
  } else {
    clauses = encoder->Encode(state);
  }
  return clauses;
}

template <class T>
std::priority_queue<typename PropagatorDynamic<T>::PropagtionElement,
                    std::vector<typename PropagatorDynamic<T>::PropagtionElement>,
                    std::greater<typename PropagatorDynamic<T>::PropagtionElement>>
PropagatorDynamic<T>::InitPropagationQueue(SolverState &state,
                                        int unit_start_index,
                                        int clause_start_index) {
  std::priority_queue<PropagtionElement, std::vector<PropagtionElement>,
                      std::greater<PropagtionElement>>
      propagation_queue;
  // add the unit clauses to the queue
  for (int i = unit_start_index;
       i < state.propagator_clausal_.clause_database_.unit_clauses_.size();
       ++i) {
    // add unit classes to the queue.
    BooleanLiteral l =
        state.propagator_clausal_.clause_database_.unit_clauses_[i];
    propagation_queue.push(PropagtionElement(l, 0, NULL, NULL));
  }
  // add the clauses that have one unassigned literal to the queue.
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
    // is currently unit, or was unit in a previous level and now in conflict
    if ((!state.assignments_.IsAssignedTrue(l1)) &&
        state.assignments_.IsAssignedFalse(l2)) {
      int max_level = state.assignments_.GetAssignmentLevel(l2.Variable());
      propagation_queue.push(PropagtionElement(l1, max_level,
                                               &state.propagator_clausal_,
                                               reinterpret_cast<uint64_t>(c)));
    } else if (state.assignments_.IsAssignedFalse(l2) &&
               state.assignments_.GetAssignmentLevel(l2.Variable()) <
                   state.assignments_.GetAssignmentLevel(l1.Variable())) {
      int index = l2 == c->literals_[0] ? 0 : 1;
      // TODO why no check for another true literal? I think it is because the default lit order of new clauses
      for (int i = 2; i < c->literals_.size_; ++i) {
        if (state.assignments_.GetAssignmentLevel(
                c->literals_[index].Variable()) <
            state.assignments_.GetAssignmentLevel(c->literals_[i].Variable())) {
          index = i;
        }
      }

      assert(index <= 1);
      int max_level = state.assignments_.GetAssignmentLevel(l2.Variable());
      propagation_queue.push(PropagtionElement(l1, max_level,
                                               &state.propagator_clausal_,
                                               reinterpret_cast<uint64_t>(c)));
    }
  }

  return propagation_queue;
}
template <class T>
void PropagatorDynamic<T>::UpdatePropagation(
    SolverState &state,
    std::priority_queue<PropagtionElement, std::vector<PropagtionElement>,
                        std::greater<PropagtionElement>>&
        propagation_queue,
    int min_var_index) {
  //  state.FullReset(); //Sometimes faster (especially for incremental TODO
  //  configure and test) return;

  int decision_level = state.GetCurrentDecisionLevel();
  // propagate the newly added clauses and vars
  int backtrack_level =
      PropagateLiterals(propagation_queue, state, min_var_index);
  RepairTrailPositions(state);
  backtrack_level = std::min(state.GetCurrentDecisionLevel(), backtrack_level);
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
}

template <class T>
void PropagatorDynamic<T>::RepairTrailPositions(SolverState &state) {
  auto it = state.GetTrailBegin();
  int position = 0;
  int decisionlevel = 0;
  while (!it.IsLast()) {
    state.assignments_.info_[it.GetData().VariableIndex()].position_on_trail =
        position;
    if (state.assignments_.GetAssignmentLevel(it.GetData().Variable()) ==
        decisionlevel + 1)
      decisionlevel++;
    assert(state.assignments_.GetAssignmentLevel(it.GetData().Variable()) ==
           decisionlevel);
    ++position;
    it.Next();
  }
}

template <class T>
bool PropagatorDynamic<T>::PropagateIncremental(
    SolverState &state, T *constraint) {

  int clause_index =
      state.propagator_clausal_.clause_database_.permanent_clauses_.size();
  int unit_index =
      state.propagator_clausal_.clause_database_.unit_clauses_.size();
  int var_index = state.assignments_.GetNumberOfVariables() + 1;

  std::vector<BooleanLiteral> reason;
  std::vector<BooleanLiteral> propagate;
  PropagateIncremental2(state, constraint, reason, propagate);

  if (propagate.empty()) {
    //    state.FullReset();
    return false;
  }

  //TODO propagate in one go.
  std::vector<std::vector<BooleanLiteral>> clauses =
      AddEncodingClauses(state, constraint);
  if (clauses.empty()) {
    assert(!constraint->encoder_->IsAdded(propagate[0]));
    std::vector<std::vector<BooleanLiteral>> propagate_clauses =
        //      constraint->encoder_->Encode(state, {propagate[0]});
        GetEncoder(constraint)->Propagate(state, reason, propagate);
  } else {
    //    state.FullReset();
    //    return true;
  }

  PropagateAddedClauses(state, unit_index, clause_index, var_index);
  return true;
}
template <class T>
void PropagatorDynamic<T>::PropagateAddedClauses(SolverState &state,
                                                 int unit_start_index,
                                                 int clause_start_index,
                                                 int var_start_index) {
  auto propagation_queue =
      InitPropagationQueue(state, unit_start_index, clause_start_index);
  UpdatePropagation(state, propagation_queue, var_start_index);
}

//TODO how should this be done?
template class PropagatorDynamic<WatchedCardinalityConstraint>;
template class PropagatorDynamic<WatchedSumConstraint>;
} // namespace Pumpkin
