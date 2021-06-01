#ifndef REASON_GENERIC_H
#define REASON_GENERIC_H

#include "../Basic Data Structures/boolean_literal.h"
#include "../Basic Data Structures/small_helper_structures.h"

namespace Pumpkin
{

class SolverState;

class ReasonGeneric
{
public:
	virtual ~ReasonGeneric() {};

	//must take into account that some literals might be set at the root level?
	virtual void RoundToOne(BooleanLiteral resolving_literal, SolverState &state) = 0;
	virtual Term operator[](size_t index) = 0;
	virtual uint64_t GetCoefficient(BooleanLiteral literal) = 0;
	virtual void MultiplyByFraction(uint64_t numerator, uint64_t denominator) = 0;
	virtual uint32_t GetRightHandSide() = 0;
	virtual size_t Size() = 0;
};

} //end Pumpkin namespace

#endif // !REASON_GENERIC_H
