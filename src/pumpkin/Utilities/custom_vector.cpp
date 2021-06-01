#include "custom_vector.h"

namespace Pumpkin
{

const BooleanLiteral * LiteralVector::begin() const
{
	return &literals_[0];
}

const BooleanLiteral * LiteralVector::end() const
{
	return &literals_[0] + Size();
}

size_t LiteralVector::Size() const
{
	return size_;
}

BooleanLiteral & LiteralVector::operator[](int index)
{
	assert(index < size_);
	return literals_[index];
}


BooleanLiteral LiteralVector::operator[](int index) const
{
	assert(index < size_);
	return literals_[index];
}

} //end Pumpkin namespace