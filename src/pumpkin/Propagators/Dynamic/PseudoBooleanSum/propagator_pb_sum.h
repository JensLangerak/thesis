//
// Created by jens on 16-03-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANSUM_PROPAGATOR_PB_SUM_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANSUM_PROPAGATOR_PB_SUM_H_

#include "watched_pb_sum_constraint.h"
#include "../../propagator_generic.h"
#include "database_pb_sum.h"
#include <queue>
#include "../propagator_dynamic.h"
namespace Pumpkin {
class SolverState;
class PropagatorPbSum : public PropagatorDynamic<WatchedPbSumConstraint, PbSumConstraint> {

public:
  explicit PropagatorPbSum(int64_t num_variables);
  ~PropagatorPbSum() override = default;

  ExplanationGeneric *ExplainLiteralPropagation(BooleanLiteral literal,
                                                SolverState &state)
  override; // returns the conjunction that forces the assignment of input
  // literal to true. Assumes the input literal is not undefined.
  ExplanationGeneric *ExplainFailure(SolverState &state)
  override; // returns the conjunction that leads to failure

  ReasonGeneric *ReasonLiteralPropagation(BooleanLiteral literal,
                                          SolverState &state) override;
  ReasonGeneric *ReasonFailure(SolverState &state) override;

  void Synchronise(SolverState &state)
  override; // after the state backtracks, it should call this synchronise
  // method which will internally set the pointer of the trail to
  // the new correct position

  DatabasePbSum sum_database_;

  void SetTrailIterator(TrailList<BooleanLiteral>::Iterator iterator)override;
  bool PropagateOneLiteral(SolverState &state) override;
  void ResetCounts();

  /// Debug function to check if the counts are correct.
  /// \param state
  /// \return
  bool CheckCounts(SolverState &state, WatchedPbSumConstraint *p_constraint);

  /// Keep track how often it has triggered a constraint. (for debugging etc.)
  int trigger_count_ = 0;
//private:
  // this is the main propagation method. Note that it will change watch lists
  // of true_literal and some other literals and enqueue assignments
  bool PropagateLiteral(BooleanLiteral true_literal,
                        SolverState &state) override;

  WatchedPbSumConstraint *failure_constraint_;
  BooleanLiteral last_propagated_;
  size_t last_index_;

  std::vector<BooleanLiteral>
  GetEncodingCause(SolverState &state, WatchedPbSumConstraint *constraint) override;

  void PropagateIncremental2(SolverState &state, WatchedPbSumConstraint *constraint, std::vector<BooleanLiteral> &reason, std::vector<BooleanLiteral> &propagate) override;

  IEncoder<PbSumConstraint> * GetEncoder(WatchedPbSumConstraint * constraint) override { return constraint->encoder_;};
  std::queue<WatchedPbSumConstraint *> add_constraints_;
  void AddScheduledEncodings(SolverState &state);
  void InitPropagation(WatchedPbSumConstraint *constraint, SolverState &state);
  bool debug_flag = false;
  bool PropagateInputs(WatchedPbSumConstraint *constraint, SolverState &state);
  void UpdateWeightTrail(WatchedPbSumConstraint *constraint,
                         BooleanLiteral true_literal, SolverState &state);
  bool EnqueTrailWeights(WatchedPbSumConstraint *constraint,
                         SolverState &state);
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANSUM_PROPAGATOR_PB_SUM_H_
