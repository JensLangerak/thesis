//
// Created by jens on 10-08-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_EXTENDEDGROUPS_PROPAGATOR_EXTENDED_GROUPS_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_EXTENDEDGROUPS_PROPAGATOR_EXTENDED_GROUPS_H_


#include "watched_extended_groups_constraint.h"
#include "database_extended_groups.h"
#include "../propagator_dynamic.h"
namespace Pumpkin {
class PropagatorExtendedGroups : public PropagatorDynamic<WatchedExtendedGroupsConstraint> {
public:
  explicit PropagatorExtendedGroups(int64_t num_variables);
  void Synchronise(SolverState &state)
  override; // after the state backtracks, it should call this synchronise
  // method which will internally set the pointer of the trail to
  // the new correct position
  // this is the main propagation method. Note that it will change watch lists
  // of true_literal and some other literals and enqueue assignments
  bool PropagateLiteral(BooleanLiteral true_literal,
                        SolverState &state) override;

  DatabaseExtendedGroups pseudo_boolean_database_;

  void GrowDatabase() override {
    pseudo_boolean_database_.watch_list_true_.Grow();
  }
  ~PropagatorExtendedGroups() = default;
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_EXTENDEDGROUPS_PROPAGATOR_EXTENDED_GROUPS_H_
