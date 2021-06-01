#include "boolean_literal.h"

#include <assert.h>
#include <math.h>

namespace Pumpkin
{

BooleanLiteral::BooleanLiteral()
	:code_(0)
{
}

BooleanLiteral::BooleanLiteral(BooleanVariable variable, bool is_positive)
	:code_(variable.index_ - 2*!is_positive*variable.index_)
{
	assert(variable.IsUndefined() == false);
}

bool BooleanLiteral::operator==(BooleanLiteral literal) const
{
	assert(literal.IsUndefined() == false && this->IsUndefined() == false);
	return this->code_ == literal.code_;
}

bool BooleanLiteral::operator!=(BooleanLiteral literal) const
{
	return this->code_ != literal.code_;
}

bool BooleanLiteral::IsPositive() const
{
	assert(IsUndefined() == false);
	return code_ > 0;
}

bool BooleanLiteral::IsNegative() const
{
	assert(IsUndefined() == false);
	return code_ < 0;
}

bool BooleanLiteral::IsUndefined() const
{
	return code_ == 0;
}

BooleanVariable BooleanLiteral::Variable() const
{
	assert(IsUndefined() == false);
	return BooleanVariable(abs(code_));
}

int BooleanLiteral::VariableIndex() const
{
	return Variable().index_;
}

BooleanLiteral BooleanLiteral::operator~() const
{
	return BooleanLiteral(Variable(), !IsPositive());
}

int BooleanLiteral::ToPositiveInteger() const
{
	return VariableIndex() * 2 + IsPositive();
}

std::string BooleanLiteral::ToString() const
{
	std::string s;
	if (IsNegative()) { s += '~'; }
	s += std::to_string(VariableIndex());	
	return s;
}

} //end Pumpkin namespace