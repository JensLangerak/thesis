//
// Created by jens on 31-10-20.
//

#include "propagator_encoder.h"
#include "../../../Utilities/boolean_literal.h"
#include "../../../Utilities/problem_specification.h"
#include "../../../Engine/solver_state.h"
namespace Pumpkin {

template <class T>
void PropagatorEncoder<T>::Encode(SolverState &state) {
  return;
}

template <class T>
PropagatorEncoder<T>::~PropagatorEncoder() {}

template <class T>
void PropagatorEncoder<T>::Encode(SolverState &state,
                          std::vector<BooleanLiteral> lits) {
  return Encode(state);
}
template <class T>
bool PropagatorEncoder<T>::UpdateMax(int max, SolverState &state) {
  return true;
}

template class  PropagatorEncoder<CardinalityConstraint>;
template class  PropagatorEncoder<PseudoBooleanConstraint>;
}