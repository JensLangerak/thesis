//
// Created by jens on 30-05-21.
//

#ifndef PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_PSEUDO_BOOLEAN_ADDER_H_
#define PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_PSEUDO_BOOLEAN_ADDER_H_

#include "../../../Utilities/problem_specification.h"
#include "../../i_constraint_adder.h"
#include "propagator_pseudo_boolean_2.h"
#include "../Encoders/i_encoder.h"
namespace Pumpkin {
class PseudoBooleanModifier : public IPseudoBooleanConstraintModifier {
public:
  explicit PseudoBooleanModifier(WatchedPseudoBooleanConstraint2 * constraint) : watched_constraint_(constraint) {};
  bool UpdateMax(int max, SolverState & state) override;

protected:
  WatchedPseudoBooleanConstraint2 * watched_constraint_;
};
class PseudoBooleanAdder : public IConstraintAdder<PseudoBooleanConstraint> {
public:
  explicit PseudoBooleanAdder(PropagatorPseudoBoolean2 * propagator) : propagator_(propagator) , encoder_factory(nullptr) {
                                                                                                   encoder_factory = nullptr;
                                                                                               } ;
  void AddConstraint(PseudoBooleanConstraint &c, SolverState &state) override;
  PseudoBooleanModifier* AddConstraintWithModifier(PseudoBooleanConstraint &c, SolverState& state) override;
  PropagatorPseudoBoolean2 * GetPropagator() override { return propagator_;}
  ~PseudoBooleanAdder() {
    delete encoder_factory;
    encoder_factory = nullptr;
  };

  IEncoder<PseudoBooleanConstraint>::IFactory *encoder_factory ;
protected:
  void AddPropagator(SolverState & state);
  bool propagator_added_= false;
  WatchedPseudoBooleanConstraint2 * CreateWatchedConstraint(PseudoBooleanConstraint constraint, SolverState & state);

public:
  PropagatorPseudoBoolean2 * propagator_; // TODO

};

} // namespace Pumpkin

#endif // PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_PSEUDO_BOOLEAN_ADDER_H_
