//
// Created by jens on 16-11-20.
//

#include "propagator_sum.h"
#include "../../Engine/solver_state.h"
#include "../Clausal/reason_clausal.h"
#include "Encoders/incremental_sequential_encoder.h"
#include "Encoders/totaliser_encoder.h"
#include "reason_cardinality_constraint.h"
#include "watch_list_cardinality.h"
#include <iostream>
namespace Pumpkin{
  PropagatorSum::PropagatorSum(int64_t num_variables)
      : PropagatorGeneric() {}

   ExplanationGeneric *
  PropagatorSum::ExplainLiteralPropagation(
      BooleanLiteral literal, SolverState & state) {
    assert(state.assignments_.GetAssignmentPropagator(literal.Variable()) ==
           this);
    uint64_t code = state.assignments_.GetAssignmentCode(literal.Variable());
    WatchedCardinalityConstraint *propagating_constraint =
        reinterpret_cast<WatchedCardinalityConstraint *>(code);
    int count = 0;
    for (BooleanLiteral l : propagating_constraint->literals_) {
      if (state.assignments_.IsAssignedTrue(l))
        count++;
    }
    std::queue<PropagatorCardinality::SpecialTrailPosition> special_position;
    auto lits = ((IncrementalSequentialEncoder *) propagating_constraint->encoder_)->previous_added_lits_;
    for (BooleanLiteral l : lits) {
      if (state.assignments_.IsAssignedTrue(l)) {
        int level = state.assignments_.GetAssignmentLevel(l.Variable());
        auto it = state.trail_.begin();
        if (level > 0)
        it = state.trail_delimiter_[level - 1];
        while (it.GetData() != l)
          it.Next();
        special_position.push(PropagatorCardinality::SpecialTrailPosition(
            l, it,level));
      }
    }

    bool found = false;
    for (BooleanLiteral l : ((IncrementalSequentialEncoder *) propagating_constraint->encoder_)->previous_added_lits_) {
      if (l == literal)
        found = true;
    }
//    if (!found) {
//      for (int i =0; i < ((IncrementalSequentialEncoder *) propagating_constraint->encoder_)->hist.size(); ++i) {
//        std::vector<BooleanLiteral> h =
//            ((IncrementalSequentialEncoder *)propagating_constraint->encoder_)
//                ->hist[i];
//        for (BooleanLiteral l : h) {
//          if (l == literal) {
//            found = true;
//          }
//        }
//      }
//    }
    assert(found);

    int test = ((IncrementalSequentialEncoder *) propagating_constraint->encoder_)->hist.size();
    int test2 = state.decision_level_;
    if (test == 37)
    int se = 2;
    state.propagator_cardinality_.AddEncoding(state, propagating_constraint);
//    AddEncoding(state, propagating_constraint);

    assert(test2== state.decision_level_);
    assert(state.assignments_.GetAssignmentPropagator(literal.Variable()) == &state.propagator_clausal_);
    code = state.assignments_.GetAssignmentCode(literal.Variable());
    TwoWatchedClause *propagating_clause = reinterpret_cast<TwoWatchedClause*>(code);
    return propagating_clause->ExplainLiteralPropagation(literal);
  }
  ExplanationGeneric *PropagatorSum::ExplainFailure(
      SolverState & state) {
    assert(false);
    return nullptr;
  }
  ReasonGeneric *PropagatorSum::ReasonLiteralPropagation(
      BooleanLiteral literal, SolverState & state) {
    assert(state.assignments_.GetAssignmentPropagator(literal.Variable()) ==
        this);
    uint64_t code = state.assignments_.GetAssignmentCode(literal.Variable());
    WatchedCardinalityConstraint *propagating_constraint =
        reinterpret_cast<WatchedCardinalityConstraint *>(code);
    state.propagator_cardinality_.AddEncoding(state, propagating_constraint);
//    AddEncoding(state, propagating_constraint);

    assert(state.assignments_.GetAssignmentPropagator(literal.Variable()) == &state.propagator_clausal_);
    code = state.assignments_.GetAssignmentCode(literal.Variable());
    TwoWatchedClause *propagating_clause = reinterpret_cast<TwoWatchedClause*>(code);

    return new ReasonClausal(propagating_clause);
  }
  ReasonGeneric *PropagatorSum::ReasonFailure(
      SolverState & state) {
    assert(false);
    return nullptr;
  }
  bool PropagatorSum::PropagateLiteral(BooleanLiteral true_literal,
                                                SolverState & state) {

    DatabaseCardinality &cardinality_database =
        state.propagator_cardinality_.cardinality_database_;
    WatchListCardinality &watch_list_true =
        cardinality_database.watch_list_sum_true;
    std::vector<WatcherCardinalityConstraint> &watchers_true =
        watch_list_true[true_literal];
    size_t end_position = 0;
    size_t current_index = 0;
    for (; current_index < watchers_true.size(); ++current_index) {
      WatchedCardinalityConstraint *constraint =
          watchers_true[current_index].constraint_;
      if (constraint->encoder_->EncodingAdded())
        continue;
      watchers_true[end_position] = watchers_true[current_index];
      ++end_position;

      int true_count = 0;
      int false_count = 0;
      for (BooleanLiteral l : constraint->literals_) {
        if (state.assignments_.IsAssignedTrue(l) &&
            state.assignments_.GetTrailPosition(l.Variable()) <=
                state.assignments_.GetTrailPosition(true_literal.Variable()))
          true_count++;
      }
      constraint->sum_count_ = true_count;
      assert(true_count <= constraint->max_);
      auto encoder = (IncrementalSequentialEncoder *)constraint->encoder_;
      for (int i = 0; i < true_count; i++) {
        if (!state.assignments_.IsAssignedFalse(
                encoder->previous_added_lits_[0])) {
          state.EnqueuePropagatedLiteral(
              encoder->previous_added_lits_[0], this,
              reinterpret_cast<uint64_t>(constraint));
        } else {
          bool res =
              state.propagator_cardinality_.AddEncoding(state, constraint);
          assert(res);
          return res;
        }
      }
    }
    return true;
  }
}
