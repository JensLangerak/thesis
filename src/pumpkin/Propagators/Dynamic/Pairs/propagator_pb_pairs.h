//
// Created by jens on 07-06-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PAIRS_PROPAGATOR_PB_PAIRS_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PAIRS_PROPAGATOR_PB_PAIRS_H_

#include "watched_pb_pairs_constraint.h"
#include "database_pb_pairs.h"
#include "../propagator_dynamic.h"
namespace Pumpkin {
class PropagatorPbPairs : public PropagatorDynamic<WatchedPbPairsConstraint> {
public:
  explicit PropagatorPbPairs(int64_t num_variables);
  void Synchronise(SolverState &state)
      override; // after the state backtracks, it should call this synchronise
  // method which will internally set the pointer of the trail to
  // the new correct position
  // this is the main propagation method. Note that it will change watch lists
  // of true_literal and some other literals and enqueue assignments
  bool PropagateLiteral(BooleanLiteral true_literal,
                        SolverState &state) override;

  DatabasePbPairs pseudo_boolean_database_;

  void GrowDatabase() override {
    pseudo_boolean_database_.watch_list_true_.Grow();
  }
  ~PropagatorPbPairs() = default;
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PAIRS_PROPAGATOR_PB_PAIRS_H_
