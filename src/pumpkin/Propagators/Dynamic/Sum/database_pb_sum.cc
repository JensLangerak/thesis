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
  AddWatchers(watched);
  return watched;
}
void DatabasePbSum::AddWatchers(WatchedPbSumConstraint *constraint) {
  for (WeightedLiteral lit : constraint->inputs_) {
    watch_list_input_.Add(lit, constraint);
  }
  for (WeightedLiteral lit : constraint->outputs_) {
    watch_list_output_.Add(WeightedLiteral(~(lit.literal), lit.weight), constraint);
  }

}
DatabasePbSum::DatabasePbSum(uint64_t num_vars) : watch_list_input_(num_vars), watch_list_output_(num_vars) {}
}