//
// Created by jens on 10-08-21.
//

#include "explanation_extended_groups.h"
#include "../../../Engine/solver_state.h"
#include "watched_extended_groups_constraint.h"
namespace Pumpkin {

void ExplanationExtendedGroups::InitExplanationExtendedGroups(
    WatchedExtendedGroupsConstraint *constraint, SolverState &state,
    ExplanationDynamicConstraint *explanation) {
  assert(constraint != nullptr);
  assert(constraint->current_sum_value_ > constraint->max_);
  //  constraint->UpdateConflictCount(state);
  auto lits_ = std::vector<BooleanLiteral>();
  // Check if the min or max constraint is violated
  bool select_value = constraint->current_sum_value_ > constraint->max_;
  int sum = 0;
  int start = 0;
  if (!constraint->auxiliary_literals_.empty()) {
    lits_.push_back(constraint->auxiliary_literals_.back());
    start = constraint->auxiliray_boundary_.back();
    assert(state.assignments_.IsAssignedTrue(lits_.back()));
  }
  for (int i = start; i < constraint->set_literals_.size(); ++i)
    lits_.push_back(constraint->set_literals_[i]);

  explanation->Init(lits_);
}
void ExplanationExtendedGroups::InitExplanationExtendedGroups(
    WatchedExtendedGroupsConstraint *constraint, SolverState &state,
    BooleanLiteral propagated_literal,
    ExplanationDynamicConstraint *explanation) {
  assert(constraint != nullptr);
  assert(state.assignments_.IsAssignedTrue(propagated_literal));
  // check if true for the minimum value is propagated or false for the upper
  // bound.
  bool propagated_value = true;
  int l_w = 0;
  WeightedLiteral propagated;
  for (auto wl : constraint->intput_liters_) {
    BooleanLiteral l = wl.literal;
    if (l == propagated_literal || ~l == propagated_literal) {
      propagated_value = l == propagated_literal;
      l_w = wl.weight;
      propagated = wl;
      break;
    }
  }
  assert(propagated_value == false);

  // get the cause for the propagation
  bool cause_value = !propagated_value;
  int propagation_level =
      state.assignments_.GetTrailPosition(propagated_literal.Variable());
  auto lits_ = std::vector<BooleanLiteral>();
  int sum = 0;

  int slack = constraint->max_ - constraint->current_sum_value_;

    int start = 0;
    if (!constraint->auxiliary_literals_.empty()) {
      for (int i = constraint->auxiliary_literals_.size() - 1; i >= 0; i--) {
        if (state.assignments_.IsAssignedTrue(
                constraint->auxiliary_literals_[i]) &&
            state.assignments_.GetTrailPosition(
                constraint->auxiliary_literals_[i].Variable()) <
                propagation_level) {

          lits_.push_back(constraint->auxiliary_literals_[i]);
          start = constraint->auxiliray_boundary_[i];
          break;
        }
      }
    }
    for (int i = start; i < constraint->set_literals_.size(); ++i) {
      if (!(state.assignments_.IsAssignedTrue(constraint->set_literals_[i]) &&
            state.assignments_.GetTrailPosition(
                constraint->set_literals_[i].Variable()) < propagation_level))
        break;
      lits_.push_back(constraint->set_literals_[i]);
    }

    constraint->tot++;
#ifdef USEFAILURE


    if (state.assignments_.GetAssignmentLevel(lits_.back()) == state.GetCurrentDecisionLevel()) {
      if ((!constraint->false_literal_.IsUndefined()) &&
          (!state.assignments_.IsAssignedFalse(constraint->false_literal_)) &&
          (constraint->false_literal_weight_ < 0 ||
           constraint->false_literal_weight_ > slack)) {
        if (constraint->not_false_literals_group_.count(~propagated_literal) ==
            0) {
          if (constraint->false_literal_weight_ < 0)
            constraint->false_literal_weight_ = slack + 1;
          if (propagated.weight < constraint->false_literal_weight_) {
            constraint->false_literal_weight_ = slack + 1;
            assert(slack + 1 <= propagated.weight);
          }
          constraint->false_literals_group_.insert(propagated_literal);
          //    lits_.push_back(constraint->false_literal_);
          std::vector<BooleanLiteral> false_clase;
          false_clase.push_back(constraint->false_literal_);
          for (int i = lits_.size() - 1; i >= 0; --i) {
            false_clase.push_back(~lits_[i]);
          }

#ifdef ADDASPERM
          auto learned_clause =
              state.propagator_clausal_.clause_database_.AddPermanentClause(
                  false_clase, state);
          auto r = state.EnqueuePropagatedLiteral(
              false_clase[0], &state.propagator_clausal_,
              reinterpret_cast<uint64_t>(
                  learned_clause)); // todo can this be done cleaner, without having to refer to how 'code' is interpreted by the propagator?
          assert(r);

#else
          TwoWatchedClause *learned_clause =
              state.AddLearnedClauseToDatabase(false_clase);
          auto r = state.EnqueuePropagatedLiteral(
              false_clase[0], &state.propagator_clausal_,
              reinterpret_cast<uint64_t>(
                  learned_clause)); // todo can this be done cleaner, without having to refer to how 'code' is interpreted by the propagator?
          assert(r);
//      auto res = state.PropagateEnqueued();
//      assert(res == NULL);
#endif

          lits_.clear();
          lits_.push_back(constraint->false_literal_);

          assert(state.assignments_.IsAssignedTrue(constraint->false_literal_));
          std::vector<BooleanLiteral> c = {~constraint->false_literal_,
                                           propagated_literal};

#ifdef ADDASPERM
          learned_clause =
              state.propagator_clausal_.clause_database_.AddPermanentClause(
                  c, state);
          r = state.EnqueuePropagatedLiteral(
              false_clase[0], &state.propagator_clausal_,
              reinterpret_cast<uint64_t>(
                  learned_clause)); // todo can this be done cleaner, without having to refer to how 'code' is interpreted by the propagator?
          assert(r);

#else
        learned_clause = state.AddLearnedClauseToDatabase(c);
        r = state.EnqueuePropagatedLiteral(
            c[0], &state.propagator_clausal_,
            reinterpret_cast<uint64_t>(
                learned_clause)); // todo can this be done cleaner, without having to refer to how 'code' is interpreted by the propagator?
        assert(r);
        res = state.PropagateEnqueued();
#endif

          constraint->used++;
        } else {
          constraint->create_new_failure_ = true;
        }
      }
    } else {
      constraint->wrong_level++;
      std::cout << "usage " << constraint->wrong_level << "   " << constraint->used << "    " << constraint->tot << std::endl;
    }
#endif

  explanation->Init(lits_);
}
} // namespace Pumpkin
