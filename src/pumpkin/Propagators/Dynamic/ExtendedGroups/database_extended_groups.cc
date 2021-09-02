//
// Created by jens on 10-08-21.
//

#include "database_extended_groups.h"
#include "../../../Utilities/problem_specification.h"
#include "../../../Engine/solver_state.h"
#include "watched_extended_groups_constraint.h"

namespace Pumpkin {

DatabaseExtendedGroups::DatabaseExtendedGroups(uint64_t num_vars) : watch_list_true_(num_vars){}
WatchedExtendedGroupsConstraint *
DatabaseExtendedGroups::AddPermanentConstraint(PseudoBooleanConstraint &constraint,
                                        SolverState &state) {
  WatchedExtendedGroupsConstraint * watched = new WatchedExtendedGroupsConstraint(constraint);
  permanent_constraints_.push_back(watched);
  AddWatchers(watched);
  return watched;
}
DatabaseExtendedGroups::~DatabaseExtendedGroups() {
  for (auto *c : permanent_constraints_) {
    for (WeightedLiteral l : c->intput_liters_) {
      watch_list_true_.Remove(l.literal, c);
    }
    delete c;
  }
}
void DatabaseExtendedGroups::AddWatchers(WatchedExtendedGroupsConstraint *constraint) {
  for (WeightedLiteral lit : constraint->intput_liters_) {
    watch_list_true_.Add(lit.literal, lit.weight, constraint);
  }
}
}