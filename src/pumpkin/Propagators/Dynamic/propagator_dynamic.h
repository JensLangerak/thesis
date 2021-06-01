//
// Created by jens on 21-05-21.
//

#ifndef PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_PROPAGATOR_DYNAMIC_H_
#define PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_PROPAGATOR_DYNAMIC_H_

#include "../propagator_generic.h"
#include "explanation_dynamic_constraint.h"
#include "../explanation_generator.h"

namespace Pumpkin {
class WatchedDynamicConstraint;
template <class Watched>
class PropagatorDynamic : public PropagatorGeneric {
public:
  ReasonGeneric *ReasonLiteralPropagation(BooleanLiteral literal,
                                          SolverState &state) override;
  ReasonGeneric *ReasonFailure(SolverState &state) override;

  ExplanationGeneric *ExplainLiteralPropagation(BooleanLiteral literal,
                                                SolverState &state)
  override; // returns the conjunction that forces the assignment of input
  // literal to true. Assumes the input literal is not undefined.

  ExplanationGeneric *ExplainFailure(SolverState &state)
  override; // returns the conjunction that leads to failure

  bool PropagateOneLiteral(SolverState &state) override;

  void Synchronise(SolverState &state) override;
  //  virtual void AddScheduledEncodings(SolverState &state);
protected:
  PropagatorDynamic() : PropagatorGeneric(), failure_constraint_(nullptr), last_propagation_info_(LastPropagationInfo(0,BooleanLiteral(),0,0)) {};
  WatchedDynamicConstraint * failure_constraint_;

  struct LastPropagationInfo {
    int propagation_step = 0;
    BooleanLiteral last_propagated;
    size_t last_watcher_index;
    size_t end_index;
    LastPropagationInfo(int propagation_step, BooleanLiteral last_propagated, size_t last_watcher_index, size_t end_index) :
      propagation_step(propagation_step), last_propagated(last_propagated), last_watcher_index(last_watcher_index), end_index(end_index) {}
    // TODO end index?
  };
  LastPropagationInfo last_propagation_info_;
  ExplanationGenerator<ExplanationDynamicConstraint> explanation_generator_;
};
} // namespace Pumpkin

#endif // PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_PROPAGATOR_DYNAMIC_H_
