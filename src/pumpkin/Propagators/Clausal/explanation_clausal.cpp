#include "explanation_clausal.h"
#include "../../Utilities/runtime_assert.h"

#include <cassert>

namespace Pumpkin
{
ExplanationClausal::ExplanationClausal()
{
	p_literals_ = NULL;
	ignore_position_ = -1;

}

ExplanationClausal::ExplanationClausal(const LiteralVector& literals)
{
	Initialise(literals);
}

ExplanationClausal::ExplanationClausal(const LiteralVector& literals, int ignore_position)
{
	Initialise(literals, ignore_position);
}

void ExplanationClausal::Initialise(const LiteralVector& literals)
{
	p_literals_ = &literals;
	ignore_position_ = int(literals.Size());
}

void ExplanationClausal::Initialise(const LiteralVector& literals, int ignore_position)
{
	runtime_assert(ignore_position == 0); //the convention is that the literal at position zero is the propagated literal. Might change in the future.

	p_literals_ = &literals;
	ignore_position_ = ignore_position;
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

size_t ExplanationClausal::Size() const
{
	runtime_assert(p_literals_ != NULL);

	return int(p_literals_->Size()) - int(ignore_position_ < int(p_literals_->Size()));
}

} //end Pumpkin namespace