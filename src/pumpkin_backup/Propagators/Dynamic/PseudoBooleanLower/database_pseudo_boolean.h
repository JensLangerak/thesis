//
// Created by jens on 10-12-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANLOWER_DATABASE_PSEUDO_BOOLEAN_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANLOWER_DATABASE_PSEUDO_BOOLEAN_H_

#include "watch_list_pseudo_boolean.h"
#include <cstdint>
#include "learnt_group.h"
#include "node_database.h"
namespace Pumpkin {
class SolverState;
class DatabasePseudoBoolean3 {
public:
  explicit DatabasePseudoBoolean3(uint64_t num_vars);

  /// Create a new WatchedCardinalityConstraint from the given constraint. Add the constraint to the watched lists.
  ///
  /// \param constraint
  /// \param state
  /// \return the constructed constraint.
  WatchedPseudoBooleanConstraint3* AddPermanentConstraint(PseudoBooleanConstraint &constraint,
                                                       SolverState &state);

  WatchListPseudoBoolean3 watch_list_true;
  std::vector<WatchedPseudoBooleanConstraint3*> permanent_constraints_;
  NodeDatabase node_database_;
//  std::vector<LearntGroup*> learnt_groups_;
//  std::map<BooleanLiteral, std::vector<LearntGroup*>> member_groups_;

  ~DatabasePseudoBoolean3();
private:
  void AddWatchers(WatchedPseudoBooleanConstraint3 *constraint);
};
} // namespace Pumpkin
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_DATABASE_PSEUDO_BOOLEAN_H_
