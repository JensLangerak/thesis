//
// Created by jens on 10-08-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_EXTENDEDGROUPS_EXPLANATION_EXTENDED_GROUPS_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_EXTENDEDGROUPS_EXPLANATION_EXTENDED_GROUPS_H_
#include "watched_extended_groups_constraint.h"
namespace Pumpkin {
class ExplanationExtendedGroups {
public:
  /// Get the explanation for the conflict
  /// \param constraint
  /// \param state
  static void InitExplanationExtendedGroups(WatchedExtendedGroupsConstraint * constraint, SolverState &state, ExplanationDynamicConstraint * explanation);
  /// Get the explanation for the propagated value.
  static void InitExplanationExtendedGroups(WatchedExtendedGroupsConstraint * constraint, SolverState &state, BooleanLiteral propagated_literal, ExplanationDynamicConstraint * explanation);


};

}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_EXTENDEDGROUPS_EXPLANATION_EXTENDED_GROUPS_H_
