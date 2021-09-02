#include "learned_pseudo_boolean_constraint_iterator.h"
#include "runtime_assert.h"

namespace Pumpkin
{

LearnedPseudoBooleanConstraintIterator::LearnedPseudoBooleanConstraintIterator(LearnedPseudoBooleanConstraint & learned_constraint):
	underlying_constraint_(learned_constraint), current_index_(0)
{
}

void LearnedPseudoBooleanConstraintIterator::operator++()
{
	runtime_assert(current_index_ < underlying_constraint_.present_literals_.size());
	current_index_++;
}

uint32_t LearnedPseudoBooleanConstraintIterator::Coefficient()
{
	runtime_assert(underlying_constraint_.GetCoefficient(Literal()) != 0);
	return underlying_constraint_.GetCoefficient(Literal());
}

BooleanLiteral LearnedPseudoBooleanConstraintIterator::Literal()
{
	return underlying_constraint_.present_literals_[current_index_];
}

BooleanVariable LearnedPseudoBooleanConstraintIterator::Variable()
{
	return Literal().Variable();
}

bool LearnedPseudoBooleanConstraintIterator::IsValid()
{
	return current_index_ != underlying_constraint_.present_literals_.size();
}

/*
bool LearnedPseudoBooleanConstraintIterator::operator==(const PseudoBooleanTermIteratorAbstract & rhs)
{
	return this->MemoryLocationPointer() == rhs.MemoryLocationPointer();
}

uint64_t LearnedPseudoBooleanConstraintIterator::MemoryLocationPointer() const
{//not sure if this is the ideal way to do it
	return reinterpret_cast<uint64_t>(&underlying_constraint_.present_literals_[0] + current_index_);
}
*/

} //end Pumpkin namespace