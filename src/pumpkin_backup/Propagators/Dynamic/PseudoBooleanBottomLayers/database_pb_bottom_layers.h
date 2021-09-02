//
// Created by jens on 18-05-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANBOTTOMLAYERS_DATABASE_PB_BOTTOM_LAYERS_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANBOTTOMLAYERS_DATABASE_PB_BOTTOM_LAYERS_H_

#include "../../../Basic Data Structures/problem_specification.h"
#include "watch_list_pb_bottom_layers.h"
#include "watched_pb_bottom_layers_constraint.h"
#include <cstdint>
#include <stdint-gcc.h>
#include <vector>
namespace Pumpkin {
class DatabasePbBottomLayers {

public:
  explicit DatabasePbBottomLayers(uint64_t num_vars);

  /// Create a new WatchedSumConstraint from the given constraint. Add the constraint to the watched lists.
  ///
  /// \param constraint
  /// \param state
  /// \return the constructed constraint.
  WatchedPbBottomLayersConstraint* AddPermanentConstraint(PseudoBooleanConstraint &constraint,
                                                 SolverState &state);

  WatchedPbBottomLayersConstraint * AddPermanentConstraint(std::vector<BooleanLiteral> input_lits, std::vector<uint32_t > input_weights, int max, SolverState &state);
  WatchListPbBottomLayers watch_list_true_;
  std::vector<WatchedPbBottomLayersConstraint*> permanent_constraints_;

  ~DatabasePbBottomLayers();
private:
  void AddWatchers(WatchedPbBottomLayersConstraint *constraint);
};
}

#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANBOTTOMLAYERS_DATABASE_PB_BOTTOM_LAYERS_H_
