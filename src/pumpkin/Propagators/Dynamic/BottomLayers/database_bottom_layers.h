//
// Created by jens on 06-07-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_BOTTOMLAYERS_DATABASE_BOTTOM_LAYERS_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_BOTTOMLAYERS_DATABASE_BOTTOM_LAYERS_H_

#include "../database_dynamic_constraint.h"
#include "../watcher_dynamic_constraint.h"
#include "../watch_list_dynamic.h"
#include "../Encoders/i_encoder.h"
#include <cstdint>

namespace Pumpkin {
class SolverState;
class PseudoBooleanConstraint;
class WatchedBottomLayersConstraint;
class DatabaseBottomLayers {//; : public DatabaseDynamicConstraint {
public:
  explicit  DatabaseBottomLayers(uint64_t num_vars);

  /// Create a new WatchedCardinalityConstraint from the given constraint. Add the constraint to the watched lists.
  ///
  /// \param constraint
  /// \param state
  /// \return the constructed constraint.
  WatchedBottomLayersConstraint* AddPermanentConstraint(PseudoBooleanConstraint &constraint,
                                                          SolverState &state);

  WatchListDynamic<WatchedBottomLayersConstraint, WatcherDynamicConstraint<WatchedBottomLayersConstraint>> watch_list_true_;
  std::vector<WatchedBottomLayersConstraint*> permanent_constraints_;

  ~DatabaseBottomLayers() = default;// override;
private:
  void AddWatchers(WatchedBottomLayersConstraint *constraint);


};
} // namespace Pumpkin

#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_BOTTOMLAYERS_DATABASE_BOTTOM_LAYERS_H_
