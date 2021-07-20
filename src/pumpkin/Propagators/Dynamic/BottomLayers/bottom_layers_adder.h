//
// Created by jens on 07-07-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_BOTTOMLAYERS_BOTTOM_LAYERS_ADDER_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_BOTTOMLAYERS_BOTTOM_LAYERS_ADDER_H_
#include "../../../Utilities/problem_specification.h"
#include "../../i_constraint_adder.h"
#include "propagator_bottom_layers.h"
#include "watched_bottom_layers_constraint.h"
namespace Pumpkin {
class BottomLayersModifier : public IPseudoBooleanConstraintModifier {
public:
  explicit BottomLayersModifier(WatchedBottomLayersConstraint *constraint)
      : watched_constraint_(constraint){};
  bool UpdateMax(int max, SolverState &state) override;

protected:
  WatchedBottomLayersConstraint *watched_constraint_;
};
class BottomLayersAdder : public IConstraintAdder<PseudoBooleanConstraint> {
public:
  explicit BottomLayersAdder(PropagatorBottomLayers *propagator)
      : propagator_(propagator) {
  };
  void AddConstraint(PseudoBooleanConstraint &c, SolverState &state) override;
  BottomLayersModifier *AddConstraintWithModifier(PseudoBooleanConstraint &c,
                                                   SolverState &state) override;
  PropagatorBottomLayers *GetPropagator() override { return propagator_; }
  ~BottomLayersAdder() {
  };


protected:
  void AddPropagator(SolverState &state);
  bool propagator_added_ = false;
  WatchedBottomLayersConstraint *
  CreateWatchedConstraint(PseudoBooleanConstraint constraint,
                          SolverState &state);
  PropagatorBottomLayers *propagator_; // TODO
};
}

#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_BOTTOMLAYERS_BOTTOM_LAYERS_ADDER_H_
