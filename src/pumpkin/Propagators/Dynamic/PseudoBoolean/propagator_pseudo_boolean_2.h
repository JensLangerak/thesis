//
// Created by jens on 24-05-21.
//

#ifndef PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_PROPAGATOR_PSEUDO_BOOLEAN_2_H_
#define PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_PROPAGATOR_PSEUDO_BOOLEAN_2_H_

#include "../propagator_dynamic.h"
#include "database_pseudo_boolean_2.h"
#include "watched_pseudo_boolean_constraint_2.h"
#include "explanation_pseudo_boolean_constraint_2.h"

namespace Pumpkin {
class SolverState;
class PropagatorPseudoBoolean2 : public PropagatorDynamic<WatchedPseudoBooleanConstraint2> {
public:
  explicit PropagatorPseudoBoolean2(int64_t num_variables);
  void Synchronise(SolverState &state)
  override; // after the state backtracks, it should call this synchronise
  // method which will internally set the pointer of the trail to
  // the new correct position
  // this is the main propagation method. Note that it will change watch lists
  // of true_literal and some other literals and enqueue assignments
  bool PropagateLiteral(BooleanLiteral true_literal,
                        SolverState &state) override;

  DatabasePseudoBoolean2 pseudo_boolean_database_;

  void GrowDatabase() override {
    pseudo_boolean_database_.watch_list_true_.Grow(); }
  ~PropagatorPseudoBoolean2() = default;
  virtual void Reset(SolverState &state);

protected:



};
} // namespace Pumpkin

#endif // PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_PROPAGATOR_PSEUDO_BOOLEAN_2_H_
