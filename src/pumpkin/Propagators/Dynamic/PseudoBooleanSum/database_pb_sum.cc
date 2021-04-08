//
// Created by jens on 12-03-21.
//

#include "database_pb_sum.h"
#include "../../../Engine/solver_state.h"
#include "../../../Basic Data Structures/problem_specification.h"

namespace Pumpkin {

DatabasePbSum::DatabasePbSum(uint64_t num_vars)
    : watch_list_true(num_vars) {}

WatchedPbSumConstraint* DatabasePbSum::AddPermanentConstraint(PbSumConstraint &constraint, SolverState & state) {
  IEncoder<PbSumConstraint> * encoder =  constraint.encoder_factory->Create(constraint);
  return AddPermanentConstraint(constraint.input_literals, constraint.input_coefficients, constraint.output_literals, constraint.output_coefficients, encoder, state);

}
WatchedPbSumConstraint *DatabasePbSum::AddPermanentConstraint(
    std::vector<BooleanLiteral> input_lits, std::vector<uint32_t> input_weights,
    std::vector<BooleanLiteral> output_lits, std::vector<uint32_t> outpu_weights,
    IEncoder<PbSumConstraint> *encoder, SolverState &state) {
  WatchedPbSumConstraint * watched = new WatchedPbSumConstraint(input_lits,input_weights, output_lits, outpu_weights, encoder);
  state.propagator_pb_sum_.InitPropagation(watched, state);
  if (watched->encoder_->EncodingAddAtStart()) {
    auto res = watched->encoder_->Encode(state);
  }
  permanent_constraints_.push_back(watched);
  AddWatchers(watched);
  return watched;
}
void DatabasePbSum::AddWatchers(WatchedPbSumConstraint *constraint) {
  for (WeightedLiteral lit : constraint->inputs_) {
    watch_list_true.Add(lit, constraint);
  }

}
DatabasePbSum::~DatabasePbSum() {
  for (WatchedPbSumConstraint *c : permanent_constraints_) {
    for (WeightedLiteral l : c->inputs_) {
      watch_list_true.Remove(l.literal, c);
    }
//    delete c;
  }

}

}
