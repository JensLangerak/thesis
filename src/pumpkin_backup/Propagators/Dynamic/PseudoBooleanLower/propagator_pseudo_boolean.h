//
// Created by jens on 10-12-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANLOWER_PROPAGATOR_PSEUDO_BOOLEAN_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANLOWER_PROPAGATOR_PSEUDO_BOOLEAN_H_

#include "../../propagator_generic.h"
#include "../propagator_dynamic.h"
#include "database_pseudo_boolean.h"
#include "watched_pseudo_boolean_constraint.h"
#include <queue>
#include <set>
namespace Pumpkin {

class PropagatorPseudoBoolean3 : public PropagatorDynamic<WatchedPseudoBooleanConstraint3, PseudoBooleanConstraint> {
public:
  explicit PropagatorPseudoBoolean3(int64_t num_variables);
  ~PropagatorPseudoBoolean3() override = default;

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

  DatabasePseudoBoolean3 pseudo_boolean_database_;

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

  WatchedPseudoBooleanConstraint3 *failure_constraint_;
  BooleanLiteral last_propagated_;
  size_t last_index_;


  std::vector<BooleanLiteral>
  GetEncodingCause(SolverState &state, WatchedPseudoBooleanConstraint3 *constraint) override;

  void PropagateIncremental2(SolverState &state, WatchedPseudoBooleanConstraint3 *constraint, std::vector<BooleanLiteral> &reason, std::vector<BooleanLiteral> &propagate) override;

  IEncoder<PseudoBooleanConstraint> * GetEncoder(WatchedPseudoBooleanConstraint3 * constraint) override { return constraint->encoder_;};
  std::queue<WatchedPseudoBooleanConstraint3 *> add_constraints_;
  void RecomputeConstraintSums(SolverState &state,
                               std::set<WatchedPseudoBooleanConstraint3 *> update_constraints);
  void AddScheduledEncodings(SolverState &state);
};
} // namespace Pumpkin

#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_PROPAGATOR_PSEUDO_BOOLEAN_H_
