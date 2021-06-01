#ifndef LEARNED_PSEUDO_BOOLEAN_CONSTRAINT_ITERATOR_H
#define LEARNED_PSEUDO_BOOLEAN_CONSTRAINT_ITERATOR_H

#include "pseudo_boolean_term_iterator_abstract.h"
#include "learned_pseudo_boolean_constraint.h"

namespace Pumpkin
{

class LearnedPseudoBooleanConstraintIterator : public PseudoBooleanTermIteratorAbstract
{
public:
	LearnedPseudoBooleanConstraintIterator(LearnedPseudoBooleanConstraint &learned_constraint);

	void operator++();
	uint32_t Coefficient();
	BooleanLiteral Literal();
	BooleanVariable Variable();
	bool IsValid();
	//bool operator==(const PseudoBooleanTermIteratorAbstract &rhs);
	
//protected:

	//uint64_t MemoryLocationPointer() const;

private:
	LearnedPseudoBooleanConstraint & underlying_constraint_;
	size_t current_index_;
};

} //end Pumpkin namespace

#endif // !LEARNED_PSEUDO_BOOLEAN_CONSTRAINT_ITERATOR_H