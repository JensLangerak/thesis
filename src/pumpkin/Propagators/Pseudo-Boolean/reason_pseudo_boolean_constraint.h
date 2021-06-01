#ifndef REASON_PSEUDO_BOOLEAN_CONSTRAINT_H
#define REASON_PSEUDO_BOOLEAN_CONSTRAINT_H

#include "../reason_generic.h"
#include "watched_pseudo_boolean_constraint.h"
#include "../../Utilities/learned_pseudo_boolean_constraint.h"

namespace Pumpkin
{

class SolverState;

//the implementation can be improved by not using 'learned pseudo boolean constraint' and instead directly manipulate the data
//but this was done for simplicity for now
class ReasonPseudoBooleanConstraint : public ReasonGeneric
{
public:
	//must take into account that some literals might be set at the root level

	ReasonPseudoBooleanConstraint(WatchedPseudoBooleanConstraint *constraint, SolverState &state);
	ReasonPseudoBooleanConstraint(WatchedPseudoBooleanConstraint *constraint, BooleanLiteral propagated_literal, SolverState &state);

	void RoundToOne(BooleanLiteral resolving_literal, SolverState &state);
	Term operator[](size_t index);
	uint64_t GetCoefficient(BooleanLiteral literal);
	void MultiplyByFraction(uint64_t numerator, uint64_t denominator);
	uint32_t GetRightHandSide();
	size_t Size();
	
private:
	LearnedPseudoBooleanConstraint pseudo_boolean_constraint_;
};

} //end Pumpkin namespace

#endif // !REASON_PSEUDO_BOOLEAN_CONSTRAINT_H