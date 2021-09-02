//
// Created by jens on 07-06-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PAIRS_PB_PAIRS_ADDER_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PAIRS_PB_PAIRS_ADDER_H_
#include "../../../Utilities/problem_specification.h"
#include "../../i_constraint_adder.h"
#include "propagator_pb_pairs.h"
#include "../Encoders/i_encoder.h"
namespace Pumpkin {


class PbPairsModifier : public IPseudoBooleanConstraintModifier {
public:
  explicit PbPairsModifier(WatchedPbPairsConstraint * constraint) : watched_constraint_(constraint) {};
  bool UpdateMax(int max, SolverState & state) override;

protected:
  WatchedPbPairsConstraint * watched_constraint_;
};
class PbPairsAdder : public IConstraintAdder<PseudoBooleanConstraint> {
public:
  explicit PbPairsAdder(PropagatorPbPairs * propagator) : propagator_(propagator) {
  } ;
  void AddConstraint(PseudoBooleanConstraint &c, SolverState &state) override;
  PbPairsModifier* AddConstraintWithModifier(PseudoBooleanConstraint &c, SolverState& state) override;
  PropagatorPbPairs * GetPropagator() override { return propagator_;}
  ~PbPairsAdder() {
  };

protected:
  void AddPropagator(SolverState & state);
  bool propagator_added_= false;
  WatchedPbPairsConstraint * CreateWatchedConstraint(PseudoBooleanConstraint constraint, SolverState & state);
  PropagatorPbPairs * propagator_; // TODO

};

}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PAIRS_PB_PAIRS_ADDER_H_
