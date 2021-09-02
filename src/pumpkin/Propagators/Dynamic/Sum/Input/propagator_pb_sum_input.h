//
// Created by jens on 18-06-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_SUM_INPUT_PROPAGATOR_PB_SUM_INPUT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_SUM_INPUT_PROPAGATOR_PB_SUM_INPUT_H_

#include "explanation_pb_sum_constraint_input.h"
#include "../watched_pb_sum_constraint.h"
#include "../../propagator_dynamic.h"
#include "../database_pb_sum.h"
namespace Pumpkin {
class PropagatorPbSumInput  : public PropagatorDynamic<WatchedPbSumConstraint> {
public:
  explicit PropagatorPbSumInput(int64_t num_variables);
  void Synchronise(SolverState &state)
  override; // after the state backtracks, it should call this synchronise
  // method which will internally set the pointer of the trail to
  // the new correct position
  // this is the main propagation method. Note that it will change watch lists
  // of true_literal and some other literals and enqueue assignments
  bool PropagateLiteral(BooleanLiteral true_literal,
                        SolverState &state) override;

  DatabasePbSum pb_sum_database_;

  void GrowDatabase() override {
    pb_sum_database_.watch_list_input_.Grow();
    pb_sum_database_.watch_list_output_.Grow();
  }
  ~PropagatorPbSumInput() = default;

  bool PropagateInputs(WatchedPbSumConstraint *constraint, SolverState &state);
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_SUM_INPUT_PROPAGATOR_PB_SUM_INPUT_H_
