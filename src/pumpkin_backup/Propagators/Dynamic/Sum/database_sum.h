//
// Created by jens on 16-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_SUM_DATABASE_SUM_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_SUM_DATABASE_SUM_H_

#include "watch_list_sum.h"
#include "watched_sum_constraint.h"
#include "../../../Basic Data Structures/problem_specification.h"
#include <stdint-gcc.h>
#include <vector>
namespace Pumpkin {
class DatabaseSum {
public:
  explicit DatabaseSum(uint64_t num_vars);

  /// Create a new WatchedSumConstraint from the given constraint. Add the constraint to the watched lists.
  ///
  /// \param constraint
  /// \param state
  /// \return the constructed constraint.
  WatchedSumConstraint* AddPermanentConstraint(SumConstraint &constraint,
                                                       SolverState &state);

  WatchListSum watch_list_true;
//  WatchListSum watch_list_false;
  std::vector<WatchedSumConstraint*> permanent_constraints_;

  ~DatabaseSum();
private:
  void AddWatchers(WatchedSumConstraint *constraint);
};
} // namespace Pumpkin
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_SUM_DATABASE_SUM_H_
