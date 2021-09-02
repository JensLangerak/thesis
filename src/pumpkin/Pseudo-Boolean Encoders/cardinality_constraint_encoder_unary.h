#pragma once

#include "../Utilities/boolean_literal.h"
#include "../Engine/solver_state.h"

#include <vector>

namespace Pumpkin
{
class CardinalityConstraintEncoderUnary
{
public:
	//[0] will be the literal to indicate the sum=1, [1] is sum=2, and so forth
	static std::vector<BooleanLiteral> EncodeSoftConstraint(std::vector<BooleanLiteral>& input_literals, SolverState& state);

};

} //end namespace Pumpkin