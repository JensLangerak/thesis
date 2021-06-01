//
// Created by jens on 31-10-20.
//

#include "propagator_encoder.h"
#include "../../../Basic Data Structures/boolean_literal.h"
#include "../../../Basic Data Structures/problem_specification.h"
namespace Pumpkin {

template <class T>
std::vector<std::vector<BooleanLiteral>>
PropagatorEncoder<T>::Encode(SolverState &state) {
  return std::vector<std::vector<BooleanLiteral>>();
}

template <class T>
PropagatorEncoder<T>::~PropagatorEncoder() {}

template <class T>
std::vector<std::vector<BooleanLiteral>>
PropagatorEncoder<T>::Encode(SolverState &state,
                          std::vector<BooleanLiteral> lits) {
  return Encode(state);
}

template class  PropagatorEncoder<CardinalityConstraint>;
template class  PropagatorEncoder<PseudoBooleanConstraint>;
}