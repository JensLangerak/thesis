//
// Created by jens on 24-05-21.
//

#include "database_pseudo_boolean_2.h"
#include "../../../Engine/solver_state.h"
#include "watched_pseudo_boolean_constraint_2.h"
#include "../../../Utilities/problem_specification.h"
namespace Pumpkin {
DatabasePseudoBoolean2::DatabasePseudoBoolean2(uint64_t num_vars)
    : watch_list_true_(num_vars) {}


WatchedPseudoBooleanConstraint2* DatabasePseudoBoolean2::AddPermanentConstraint(PseudoBooleanConstraint &constraint, SolverState & state) {
  WatchedPseudoBooleanConstraint2 * watched = new WatchedPseudoBooleanConstraint2(constraint);
  if (watched->encoder_->EncodingAddAtStart()) {
    watched->encoder_->Encode(state);
  }
  permanent_constraints_.push_back(watched);
  AddWatchers(watched);
  return watched;
}
void DatabasePseudoBoolean2::AddWatchers(WatchedPseudoBooleanConstraint2 *constraint) {
  for (WeightedLiteral lit : constraint->current_literals_) {
    watch_list_true_.Add(lit.literal, lit.weight, constraint);
  }

}
DatabasePseudoBoolean2::~DatabasePseudoBoolean2() {
  for (WatchedPseudoBooleanConstraint2 *c : permanent_constraints_) {
    for (WeightedLiteral l : c->current_literals_) {
      watch_list_true_.Remove(l.literal, c);
    }
    delete c;
  }

}
}
