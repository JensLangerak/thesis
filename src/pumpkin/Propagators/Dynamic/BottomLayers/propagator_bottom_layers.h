//
// Created by jens on 06-07-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_BOTTOMLAYERS_PROPAGATOR_BOTTOM_LAYERS_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_BOTTOMLAYERS_PROPAGATOR_BOTTOM_LAYERS_H_
#include "watched_bottom_layers_constraint.h"
#include "database_bottom_layers.h"
#include "../propagator_dynamic.h"
namespace Pumpkin {
class PropagatorBottomLayers : public PropagatorDynamic<WatchedBottomLayersConstraint> {
public:
  explicit PropagatorBottomLayers(int64_t num_variables);
  void Synchronise(SolverState &state)
      override; // after the state backtracks, it should call this synchronise
  // method which will internally set the pointer of the trail to
  // the new correct position
  // this is the main propagation method. Note that it will change watch lists
  // of true_literal and some other literals and enqueue assignments
  bool PropagateLiteral(BooleanLiteral true_literal,
                        SolverState &state) override;

  DatabaseBottomLayers database_bottom_layers_;

  void GrowDatabase() override {
    database_bottom_layers_.watch_list_true_.Grow();
  }
  ~PropagatorBottomLayers() = default;
};

}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_BOTTOMLAYERS_PROPAGATOR_BOTTOM_LAYERS_H_
