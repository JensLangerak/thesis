//
// Created by jens on 10-12-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_DATABASE_PSEUDO_BOOLEAN_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_DATABASE_PSEUDO_BOOLEAN_H_

#include "watch_list_pseudo_boolean.h"
#include <cstdint>
namespace Pumpkin {
class SolverState;
class DatabasePseudoBoolean2 {
public:
  explicit DatabasePseudoBoolean2(uint64_t num_vars);

  /// Create a new WatchedCardinalityConstraint from the given constraint. Add the constraint to the watched lists.
  ///
  /// \param constraint
  /// \param state
  /// \return the constructed constraint.
  WatchedPseudoBooleanConstraint2* AddPermanentConstraint(PseudoBooleanConstraint &constraint,
                                                       SolverState &state);

  WatchListPseudoBoolean2 watch_list_true;
  std::vector<WatchedPseudoBooleanConstraint2*> permanent_constraints_;

  ~DatabasePseudoBoolean2();
private:
  void AddWatchers(WatchedPseudoBooleanConstraint2 *constraint);
};
} // namespace Pumpkin
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_DATABASE_PSEUDO_BOOLEAN_H_
