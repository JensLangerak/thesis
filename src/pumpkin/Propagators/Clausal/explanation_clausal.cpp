#include "explanation_clausal.h"

#include <cassert>

namespace Pumpkin
{

ExplanationClausal::ExplanationClausal(const LiteralVector& literals)
	:p_literals_(&literals), ignore_position_(int(literals.Size()))
{
}

ExplanationClausal::ExplanationClausal(const LiteralVector& literals, int ignore_position)
	:p_literals_(&literals), ignore_position_(ignore_position)
{
	assert(ignore_position >= 0);
}

void ExplanationClausal::Clear()
{
	p_literals_ = NULL;
	ignore_position_ = 0;
}

BooleanLiteral ExplanationClausal::operator[](int index) const
{
	assert(p_literals_ != NULL);
	if (index < ignore_position_)
	{
		return ~p_literals_->operator[](index);
	}
	else
	{
		return ~p_literals_->operator[](index+1);
	}
}

ExplanationClausal & ExplanationClausal::operator=(const ExplanationClausal &e)
{
	p_literals_ = e.p_literals_;
	ignore_position_ = e.ignore_position_;
	return *this;
}

size_t ExplanationClausal::Size() const
{
	assert(p_literals_ != NULL);

	if (p_literals_ == NULL) { return 0; }
	return int(p_literals_->Size()) - int(ignore_position_ < int(p_literals_->Size()));
}

} //end Pumpkin namespace