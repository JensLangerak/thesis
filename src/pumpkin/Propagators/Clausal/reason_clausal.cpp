#include "reason_clausal.h"

namespace Pumpkin
{

ReasonClausal::ReasonClausal(TwoWatchedClause * clause):
	explanation_(0),
	scaling_factor_(1)
{
	explanation_ = clause->ExplainFailure();
}

/*ReasonClausal::ReasonClausal(TwoWatchedClause * clause, BooleanLiteral propagated_literal):
	explanation_(0)
{
	assert(clause->literals_[0] == propagated_literal);
	explanation_ = clause->ExplainLiteralPropagation(propagated_literal);
}*/

ReasonClausal::~ReasonClausal()
{
	delete explanation_;
}

void ReasonClausal::RoundToOne(BooleanLiteral resolving_literal, SolverState & state)
{
	return; //does nothing for clauses
}

Term ReasonClausal::operator[](size_t index)
{
	return Term(~(*explanation_)[index], scaling_factor_); 
}

uint64_t ReasonClausal::GetCoefficient(BooleanLiteral literal)
{
	//could be done more efficiently...
	for (size_t i = 0; i < Size(); i++)
	{
		Term term = operator[](i);
		if (term.literal == literal) { return term.coefficient; }
	}
	return 0;
}

void ReasonClausal::MultiplyByFraction(uint64_t numerator, uint64_t denominator)
{
	assert(denominator == 1);
	scaling_factor_ = numerator;
}

uint32_t ReasonClausal::GetRightHandSide()
{
	return 1;
}

size_t ReasonClausal::Size()
{
	return explanation_->Size();
}

} //end Pumpkin namespace