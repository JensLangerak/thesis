#ifndef REASON_CLAUSAL_H_
#define REASON_CLAUSAL_H_

#include "../reason_generic.h"
#include "explanation_clausal.h"
#include "two_watched_clause.h"

namespace Pumpkin
{

//this clause boils down to a wrapper around standard clausal explanations
class ReasonClausal : public ReasonGeneric
{
public:
	ReasonClausal(TwoWatchedClause *clause);
	ReasonClausal(TwoWatchedClause *clause, BooleanLiteral propagated_literal);

	~ReasonClausal();

	void RoundToOne(BooleanLiteral resolving_literal, SolverState &state); //does nothing for clauses
	Term operator[](size_t index);
	uint64_t GetCoefficient(BooleanLiteral literal);
	void MultiplyByFraction(uint64_t numerator, uint64_t denominator);
	uint32_t GetRightHandSide();
	size_t Size();

private:
	ExplanationClausal * explanation_;
	uint64_t scaling_factor_; //in cutting planes resolution, this might change to >1 to make sure resolution cancels a literal

};

} //end Pumpkin namespace

#endif // !REASON_CLAUSAL_H_
