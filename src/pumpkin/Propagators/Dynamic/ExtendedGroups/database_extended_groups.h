//
// Created by jens on 10-08-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_EXTENDEDGROUPS_DATABASE_EXTENDED_GROUPS_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_EXTENDEDGROUPS_DATABASE_EXTENDED_GROUPS_H_
#include <vector>
#include "../database_dynamic_constraint.h"
#include "../watcher_dynamic_constraint.h"
#include "../watch_list_dynamic.h"
#include "../Encoders/i_encoder.h"
#include <stdint-gcc.h>

namespace Pumpkin {
class SolverState;
class PseudoBooleanConstraint;
class WatchedExtendedGroupsConstraint;
class DatabaseExtendedGroups {
public:
  explicit DatabaseExtendedGroups(uint64_t num_vars);

  /// Create a new WatchedCardinalityConstraint from the given constraint. Add the constraint to the watched lists.
  ///
  /// \param constraint
  /// \param state
  /// \return the constructed constraint.
  WatchedExtendedGroupsConstraint* AddPermanentConstraint(PseudoBooleanConstraint &constraint,
                                                   SolverState &state);

  WatchListDynamic<WatchedExtendedGroupsConstraint, WatcherDynamicConstraint<WatchedExtendedGroupsConstraint>> watch_list_true_;

  std::vector<WatchedExtendedGroupsConstraint*> permanent_constraints_;

  ~DatabaseExtendedGroups();
private:
  void AddWatchers(WatchedExtendedGroupsConstraint *constraint);
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_EXTENDEDGROUPS_DATABASE_EXTENDED_GROUPS_H_
