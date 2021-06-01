#include "cardinality_constraint_encoder_unary.h"
#include "EncoderGeneralisedTotaliserCP19_2.h"

namespace Pumpkin
{
std::vector<BooleanLiteral> CardinalityConstraintEncoderUnary::EncodeSoftConstraint(std::vector<BooleanLiteral>& input_literals, SolverState& state)
{
	runtime_assert(input_literals.size() > 0);
	
	if (input_literals.size() == 1)
	{
		return std::vector<BooleanLiteral>(1, ~input_literals[0]);
	}

	GeneralisedTotaliserCP192 pb_encoder;
	pb_encoder._hax_state = &state;
	for (BooleanLiteral objective_literal : input_literals)
	{
		pb_encoder.objective_literals.push_back(PairWeightLiteral(~objective_literal, 1));
	}
	pb_encoder.ReduceRightHandSide(input_literals.size());
	runtime_assert(pb_encoder.partial_sum_literals.size() == input_literals.size());

	std::vector<BooleanLiteral> encoded_literals;
	//encoded_literals.push_back(BooleanLiteral());//todo hax fix this, we are assuming that the zeroth literal will be ignored
	//for (auto iter = pb_encoder.partial_sum_literals.rbegin(); iter != pb_encoder.partial_sum_literals.rend(); ++iter)
	for (auto iter = pb_encoder.partial_sum_literals.begin(); iter != pb_encoder.partial_sum_literals.end(); ++iter)
	{
		encoded_literals.push_back(iter->literal);
	}

    return encoded_literals;
}
}