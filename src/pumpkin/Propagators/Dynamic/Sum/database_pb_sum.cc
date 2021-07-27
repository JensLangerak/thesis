//
// Created by jens on 18-06-21.
//

#include "database_pb_sum.h"
#include "watched_pb_sum_constraint.h"
#include "../../../Utilities/problem_specification.h"
#include "../../../Engine/solver_state.h"
#include "../Encoders/i_encoder.h"

namespace Pumpkin {
WatchedPbSumConstraint* DatabasePbSum::AddPermanentConstraint(PbSumConstraint &constraint, IEncoder<PbSumConstraint>::IFactory *encoding_factory, SolverState & state) {
  WatchedPbSumConstraint * watched = new WatchedPbSumConstraint(constraint, encoding_factory);
  if (watched->encoder_->EncodingAddAtStart()) {
    watched->encoder_->Encode(state);
  }
  permanent_constraints_.push_back(watched);
  AddWatchers(watched, state);
  return watched;
}
void DatabasePbSum::AddWatchers(WatchedPbSumConstraint *constraint, SolverState &state) {
  for (WeightedLiteral lit : constraint->inputs_) {
    watch_list_input_.Add(lit, constraint);
    if (state.assignments_.IsAssignedTrue(lit.literal)) {
      UpdateInput(constraint, lit);
    }
  }
  for (WeightedLiteral lit : constraint->outputs_) {
    watch_list_output_.Add(WeightedLiteral(~(lit.literal), lit.weight), constraint);
    if (state.assignments_.IsAssignedFalse(lit.literal)) {
      UpdateOutput(constraint, lit);
    }
  }

  CheckConstraint(constraint, state);

}
DatabasePbSum::DatabasePbSum(uint64_t num_vars) : watch_list_input_(num_vars), watch_list_output_(num_vars) {}
void DatabasePbSum::UpdateInput(WatchedPbSumConstraint *constraint,
                                WeightedLiteral weighted_lit) {
  constraint->set_literals_.push_back(weighted_lit.literal);
  int index = constraint->lit_to_input_index_[weighted_lit.literal];
  int w = constraint->inputs_[index].weight;
  constraint->true_count_ +=w;


}
void DatabasePbSum::UpdateOutput(WatchedPbSumConstraint *constraint,
                                 WeightedLiteral weighted_lit) {
  int index = constraint->lit_to_output_index_[weighted_lit.literal];
  WeightedLiteral wl = constraint->outputs_[index];
  if (wl.weight < constraint->current_max_) {
    constraint->set_outputs_.push_back(wl);
    constraint->current_max_ = wl.weight - 1;
  }

}
void DatabasePbSum::CheckConstraint(WatchedPbSumConstraint *constraint,
                                    SolverState &state) {
  bool success = constraint->current_max_ >= constraint->true_count_;
  if (constraint->current_max_ < constraint->true_count_)
    throw  "Error should not ";
  for (WeightedLiteral o : constraint->outputs_) {
    if (o.weight <= constraint->true_count_) {
      if (!state.assignments_.IsAssignedTrue(o.literal)) {
        state.EnqueueDecisionLiteral(o.literal);
//        throw "Error, unexep";
      }
    }
  }

  int max_w = constraint->current_max_ - constraint->true_count_;
  if (max_w >= constraint->max_weight_)
    return;

  for (WeightedLiteral il : constraint->inputs_) {
    if (!state.assignments_.IsAssigned(il.literal)) {
      if (max_w < il.weight) {
        if (!state.assignments_.IsAssignedFalse(il.literal)) {
          state.EnqueueDecisionLiteral((~il.literal));
//          throw "Error sfsg";
        }
      }
    }
  }

}
}