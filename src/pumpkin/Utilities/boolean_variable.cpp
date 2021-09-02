#include "boolean_variable.h"

#include <assert.h>

namespace Pumpkin
{

BooleanVariable::BooleanVariable()
	:index_(0)
{
}

BooleanVariable::BooleanVariable(long index)
	: index_(index)
{
	assert(index_ > 0);
}

BooleanVariable & BooleanVariable::operator=(BooleanVariable variable)
{
	this->index_ = variable.index_;
	return *this;
}

bool BooleanVariable::IsUndefined() const
{
	return index_ == 0;
}

bool BooleanVariable::operator==(BooleanVariable variable) const
{
	return index_ == variable.index_;
}

bool BooleanVariable::operator!=(BooleanVariable variable) const
{
	return index_ != variable.index_;
}

size_t BooleanVariable::ToPositiveInteger() const
{
	return index_;
}

} //end Pumpkin namespace