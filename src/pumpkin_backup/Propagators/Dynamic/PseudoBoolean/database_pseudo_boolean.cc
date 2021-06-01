//
// Created by jens on 10-12-20.
//

#include "database_pseudo_boolean.h"

#include "../../../Engine/solver_state.h"
#include "watched_pseudo_boolean_constraint.h"
namespace Pumpkin {

DatabasePseudoBoolean2::DatabasePseudoBoolean2(uint64_t num_vars)
    : watch_list_true(num_vars) {}


WatchedPseudoBooleanConstraint2* DatabasePseudoBoolean2::AddPermanentConstraint(PseudoBooleanConstraint &constraint, SolverState & state) {
  WatchedPseudoBooleanConstraint2 * watched = new WatchedPseudoBooleanConstraint2(constraint.literals, constraint.coefficients, constraint.right_hand_side, constraint.encoder_factory->Create(constraint));
  if (watched->encoder_->EncodingAddAtStart()) {
    auto res = watched->encoder_->Encode(state);
  }
  permanent_constraints_.push_back(watched);
  AddWatchers(watched);
  return watched;
}
void DatabasePseudoBoolean2::AddWatchers(WatchedPseudoBooleanConstraint2 *constraint) {
  for (WeightedLiteral lit : constraint->current_literals_) {
    watch_list_true.Add(lit.literal, lit.weight, constraint);
  }

}
DatabasePseudoBoolean2::~DatabasePseudoBoolean2() {
  for (WatchedPseudoBooleanConstraint2 *c : permanent_constraints_) {
    for (WeightedLiteral l : c->current_literals_) {
      watch_list_true.Remove(l.literal, c);
    }
    delete c;
  }

}
} // namespace Pumpkin