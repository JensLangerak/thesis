//
// Created by jens on 16-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_DATABASE_CARDINALITY_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_DATABASE_CARDINALITY_H_

#include "watch_list_cardinality.h"
#include "watched_cardinality_constraint.h"
#include "../../Basic Data Structures/problem_specification.h"
#include <stdint-gcc.h>
#include <vector>
namespace Pumpkin {
class DatabaseCardinality {
public:
  explicit DatabaseCardinality(uint64_t num_vars);

  /// Create a new WatchedCardinalityConstraint from the given constraint. Add the constraint to the watched lists.
  ///
  /// \param constraint
  /// \param state
  /// \return the constructed constraint.
  WatchedCardinalityConstraint* AddPermanentConstraint(CardinalityConstraint &constraint,
                              SolverState &state);

  WatchListCardinality watch_list_true;
  WatchListCardinality watch_list_sum_true;
//  WatchListCardinality watch_list_false;
  std::vector<WatchedCardinalityConstraint*> permanent_constraints_;

  ~DatabaseCardinality();
private:
  void AddWatchers(WatchedCardinalityConstraint *constraint);
};
} // namespace Pumpkin
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_DATABASE_CARDINALITY_H_
