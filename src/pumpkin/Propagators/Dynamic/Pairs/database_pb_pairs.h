//
// Created by jens on 07-06-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PAIRS_DATABASE_PB_PAIRS_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PAIRS_DATABASE_PB_PAIRS_H_

#include "watched_pb_pairs_constraint.h"
#include "../watch_list_dynamic.h"
#include "../../../Utilities/problem_specification.h"
#include "../../../Engine/solver_state.h"
#include "pairs_database.h"
#include <stdint-gcc.h>
namespace Pumpkin {
class DatabasePbPairs {
  explicit DatabasePbPairs(uint64_t num_vars);

  /// Create a new WatchedCardinalityConstraint from the given constraint. Add the constraint to the watched lists.
  ///
  /// \param constraint
  /// \param state
  /// \return the constructed constraint.
  WatchedPbPairsConstraint* AddPermanentConstraint(PseudoBooleanConstraint &constraint,
                                                          SolverState &state);

  WatchListDynamic<WatchedPbPairsConstraint, WatcherDynamicConstraint<WatchedPbPairsConstraint>> watch_list_true_;

  std::vector<WatchedPbPairsConstraint*> permanent_constraints_;
  PairsDatabase pairs_database_;

  ~DatabasePbPairs();
private:
  void AddWatchers(WatchedPbPairsConstraint *constraint);
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PAIRS_DATABASE_PB_PAIRS_H_
