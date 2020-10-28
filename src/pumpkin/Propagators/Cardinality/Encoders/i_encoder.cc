//
// Created by jens on 28-10-20.
//
#include "i_encoder.h"
#include "../../../Engine/solver_state.h"
namespace Pumpkin {
IEncoder::~IEncoder() {}
std::vector<std::vector<BooleanLiteral>>
IEncoder::Encode(SolverState &state, std::vector<BooleanLiteral> lits) {
    return Encode(state);
}
}