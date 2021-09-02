//
// Created by jens on 25-05-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANBOTTOMLAYERS_PROPAGATOR_BOTTOM_LAYERS_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANBOTTOMLAYERS_PROPAGATOR_BOTTOM_LAYERS_H_

#include "database_pb_bottom_layers.h"
#include "explanation_pb_bottom_layers.h"
#include "reason_pb_bottom_layers.h"
#include "watched_pb_bottom_layers_constraint.h"
#include "../propagator_dynamic.h"
namespace Pumpkin {
class PropagatorBottomLayers : public PropagatorDynamic<WatchedPbBottomLayersConstraint, PseudoBooleanConstraint> {
public:
  explicit PropagatorBottomLayers(int64_t num_variables);
  ~PropagatorBottomLayers() override = default;

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

  DatabasePbBottomLayers pseudo_boolean_database_;

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

  WatchedPbBottomLayersConstraint *failure_constraint_;
  BooleanLiteral last_propagated_;
  size_t last_index_;


  std::vector<BooleanLiteral>
  GetEncodingCause(SolverState &state, WatchedPbBottomLayersConstraint *constraint) override;

  void PropagateIncremental2(SolverState &state, WatchedPbBottomLayersConstraint *constraint, std::vector<BooleanLiteral> &reason, std::vector<BooleanLiteral> &propagate) override;

  IEncoder<PseudoBooleanConstraint> * GetEncoder(WatchedPbBottomLayersConstraint * constraint) override { return nullptr;};
  std::queue<WatchedPbBottomLayersConstraint *> add_constraints_;
  void AddScheduledEncodings(SolverState &state);

  void CheckCount(SolverState &state, WatchedPbBottomLayersConstraint *c);
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANBOTTOMLAYERS_PROPAGATOR_BOTTOM_LAYERS_H_
