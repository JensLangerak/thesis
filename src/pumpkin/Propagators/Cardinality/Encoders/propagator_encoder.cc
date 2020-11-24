//
// Created by jens on 31-10-20.
//

#include "propagator_encoder.h"
#include "../../../Basic Data Structures/boolean_literal.h"
namespace Pumpkin {

std::vector<std::vector<BooleanLiteral>>
PropagatorEncoder::Encode(SolverState &state) {
  return std::vector<std::vector<BooleanLiteral>>();
}
PropagatorEncoder::~PropagatorEncoder() {}
std::vector<std::vector<BooleanLiteral>>
PropagatorEncoder::Encode(SolverState &state,
                          std::vector<BooleanLiteral> lits) {
  return Encode(state);
}
}