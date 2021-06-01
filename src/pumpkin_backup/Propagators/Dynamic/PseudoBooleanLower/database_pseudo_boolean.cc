//
// Created by jens on 10-12-20.
//

#include "database_pseudo_boolean.h"

#include "../../../Engine/solver_state.h"
#include "watched_pseudo_boolean_constraint.h"
namespace Pumpkin {

DatabasePseudoBoolean3::DatabasePseudoBoolean3(uint64_t num_vars)
    : watch_list_true(num_vars) {}


WatchedPseudoBooleanConstraint3* DatabasePseudoBoolean3::AddPermanentConstraint(PseudoBooleanConstraint &constraint, SolverState & state) {
  WatchedPseudoBooleanConstraint3 * watched = new WatchedPseudoBooleanConstraint3(constraint.literals, constraint.coefficients, constraint.right_hand_side, constraint.encoder_factory->Create(constraint));
//  if (watched->encoder_->EncodingAddAtStart()) {
//    auto res = watched->encoder_->Encode(state);
//  }
  permanent_constraints_.push_back(watched);
  AddWatchers(watched);
  return watched;
}
void DatabasePseudoBoolean3::AddWatchers(WatchedPseudoBooleanConstraint3 *constraint) {
  for (WeightedLiteral lit : constraint->current_literals_) {
    watch_list_true.Add(lit.literal, lit.weight, constraint);
  }

}
DatabasePseudoBoolean3::~DatabasePseudoBoolean3() {
  for (WatchedPseudoBooleanConstraint3 *c : permanent_constraints_) {
    for (WeightedLiteral l : c->current_literals_) {
      watch_list_true.Remove(l.literal, c);
    }
    delete c;
  }

}
} // namespace Pumpkin