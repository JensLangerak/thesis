//
// Created by jens on 16-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_SUM_PROPAGATOR_SUM_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_SUM_PROPAGATOR_SUM_H_

#include "../../propagator_generic.h"
#include "database_sum.h"
#include "watched_sum_constraint.h"
#include <queue>
#include "../propagator_dynamic.h"

namespace Pumpkin {

class SolverState;

class PropagatorSum :  public PropagatorDynamic<WatchedSumConstraint, SumConstraint> {
public:
  explicit PropagatorSum(int64_t num_variables);
  ~PropagatorSum() override = default;

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

  DatabaseSum sum_database_;

  void SetTrailIterator(TrailList<BooleanLiteral>::Iterator iterator)override;
  bool PropagateOneLiteral(SolverState &state) override;
  void ResetCounts();

  /// Debug function to check if the counts are correct.
  /// \param state
  /// \return
  bool CheckCounts(SolverState &state);

  /// Keep track how often it has triggered a constraint. (for debugging etc.)
  int trigger_count_ = 0;
//private:
  // this is the main propagation method. Note that it will change watch lists
  // of true_literal and some other literals and enqueue assignments
  bool PropagateLiteral(BooleanLiteral true_literal,
                        SolverState &state) override;

  WatchedSumConstraint *failure_constraint_;
  BooleanLiteral last_propagated_;
  size_t last_index_;

  std::vector<BooleanLiteral>
  GetEncodingCause(SolverState &state, WatchedSumConstraint *constraint) override;

  void PropagateIncremental2(SolverState &state, WatchedSumConstraint *constraint, std::vector<BooleanLiteral> &reason, std::vector<BooleanLiteral> &propagate) override;

  IEncoder<SumConstraint> * GetEncoder(WatchedSumConstraint * constraint) override { return constraint->encoder_;};
};
} // namespace Pumpkin
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_PROPAGATOR_CARDINALITY_H_
