//
// Created by jens on 18-06-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_SUM_DATABASE_PB_SUM_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_SUM_DATABASE_PB_SUM_H_

#include "../watcher_dynamic_constraint.h"
#include "../Encoders/i_encoder.h"
#include "../watch_list_dynamic.h"
#include <cstdint>
#include <vector>
namespace Pumpkin {
class SolverState;
class PbSumConstraint;
class WatchedPbSumConstraint;
class DatabasePbSum { //; : public DatabaseDynamicConstraint {
public:
  explicit DatabasePbSum(uint64_t num_vars);

  /// Create a new WatchedCardinalityConstraint from the given constraint. Add the constraint to the watched lists.
  ///
  /// \param constraint
  /// \param state
  /// \return the constructed constraint.
  WatchedPbSumConstraint *AddPermanentConstraint(
      PbSumConstraint &constraint,
      IEncoder<PbSumConstraint>::IFactory *encoding_factory,
      SolverState &state);

  WatchListDynamic<WatchedPbSumConstraint,
                   WatcherDynamicConstraint<WatchedPbSumConstraint>>
      watch_list_input_;
  WatchListDynamic<WatchedPbSumConstraint,
                   WatcherDynamicConstraint<WatchedPbSumConstraint>>
      watch_list_output_;
  std::vector<WatchedPbSumConstraint *> permanent_constraints_;

  ~DatabasePbSum() = default; // override;
private:
  void AddWatchers(WatchedPbSumConstraint *constraint, SolverState &state);
  void UpdateInput(WatchedPbSumConstraint *constraint,
                   WeightedLiteral weighted_lit);
  void UpdateOutput(WatchedPbSumConstraint *constraint,
                    WeightedLiteral weighted_lit);
  void CheckConstraint(WatchedPbSumConstraint *constraint, SolverState &state);
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_SUM_DATABASE_PB_SUM_H_
