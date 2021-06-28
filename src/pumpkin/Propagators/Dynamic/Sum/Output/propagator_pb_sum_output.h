//
// Created by jens on 18-06-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_SUM_OUTPUT_PROPAGATOR_PB_SUM_OUTPUT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_SUM_OUTPUT_PROPAGATOR_PB_SUM_OUTPUT_H_

#include "explanation_pb_sum_constraint_output.h"
#include "../watched_pb_sum_constraint.h"
#include "../../propagator_dynamic.h"
#include "../database_pb_sum.h"
namespace Pumpkin {
class PropagatorPbSumOutput  : public PropagatorDynamic<WatchedPbSumConstraint> {
public:
  explicit PropagatorPbSumOutput(int64_t num_variables) : pb_sum_database_(nullptr) {};
  void Synchronise(SolverState &state)
  override; // after the state backtracks, it should call this synchronise
  // method which will internally set the pointer of the trail to
  // the new correct position
  // this is the main propagation method. Note that it will change watch lists
  // of true_literal and some other literals and enqueue assignments
  bool PropagateLiteral(BooleanLiteral true_literal,
                        SolverState &state) override;

  DatabasePbSum * pb_sum_database_; //TODO

  void GrowDatabase() override {
  }
  ~PropagatorPbSumOutput() override = default;

  bool PropagateInputs(WatchedPbSumConstraint *constraint, SolverState &state);
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_SUM_OUTPUT_PROPAGATOR_PB_SUM_OUTPUT_H_
