//
// Created by jens on 18-05-21.
//


#include "database_pb_bottom_layers.h"
#include "../../../Engine/solver_state.h"
#include "../../../Basic Data Structures/problem_specification.h"

namespace Pumpkin {

DatabasePbBottomLayers::DatabasePbBottomLayers(uint64_t num_vars)
    : watch_list_true_(num_vars) {}

WatchedPbBottomLayersConstraint* DatabasePbBottomLayers::AddPermanentConstraint(PseudoBooleanConstraint &constraint, SolverState & state) {
//  IEncoder<PbSumConstraint> * encoder =  constraint.encoder_factory->Create(constraint);
  return AddPermanentConstraint(constraint.literals, constraint.coefficients,constraint.right_hand_side, state);

}
WatchedPbBottomLayersConstraint *DatabasePbBottomLayers::AddPermanentConstraint(
    std::vector<BooleanLiteral> input_lits, std::vector<uint32_t> input_weights,
   int max, SolverState &state) {
  WatchedPbBottomLayersConstraint * watched = new WatchedPbBottomLayersConstraint(input_lits,input_weights, max);
  watched->EncodeBottomLayers(state, 1);

  permanent_constraints_.push_back(watched);
  AddWatchers(watched);
  return watched;
}
void DatabasePbBottomLayers::AddWatchers(WatchedPbBottomLayersConstraint *constraint) {
  for (auto n : constraint->nodes_) {
    for (WeightedLiteral lit : n.sum_literals) {
      watch_list_true_.Add(lit, constraint);
    }
  }

}
DatabasePbBottomLayers::~DatabasePbBottomLayers() {
  for (WatchedPbBottomLayersConstraint *c : permanent_constraints_) {
    for (WeightedLiteral l : c->inputs_) {
      watch_list_true_.Remove(l.literal, c);
    }
//    delete c;
  }

}
}