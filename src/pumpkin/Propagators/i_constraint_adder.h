//
// Created by jens on 30-05-21.
//

#ifndef PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_I_CONSTRAINT_ADDER_H_
#define PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_I_CONSTRAINT_ADDER_H_

//#include "propagator_generic.h"
namespace Pumpkin {
class SolverState;
class PropagatorGeneric;
struct PseudoBooleanConstraint;
class IPseudoBooleanConstraintModifier {
public:
  virtual bool UpdateMax(int max, SolverState&state) = 0;
};
template <class C>
class IConstraintAdder {
public:
  virtual ~IConstraintAdder() = default;
  virtual void AddConstraint(C &constraint, SolverState& state) =0;
  virtual IPseudoBooleanConstraintModifier * AddConstraintWithModifier(PseudoBooleanConstraint &constraint, SolverState& state) = 0;
  virtual PropagatorGeneric * GetPropagator() = 0;
};

} // namespace Pumpkin

#endif // PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_I_CONSTRAINT_ADDER_H_
