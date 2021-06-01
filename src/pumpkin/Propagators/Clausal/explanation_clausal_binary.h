#pragma once

#include "../explanation_generic.h"

namespace Pumpkin
{

class ExplanationClausalBinary : public ExplanationGeneric
{
public:
	ExplanationClausalBinary();
	ExplanationClausalBinary(BooleanLiteral literal); //'literal' is the sole reason for the explanation (note: not its negation)
	ExplanationClausalBinary(BooleanLiteral literal1, BooleanLiteral literal2);//the two input literals, when viewed as a conjunction, are the explanation (note: not their negation)

	void Initialise(BooleanLiteral literal);
	void Initialise(BooleanLiteral literal1, BooleanLiteral literal2);

	BooleanLiteral operator[](int index) const;
	size_t Size() const;
private:
	BooleanLiteral literal1, literal2; //the second literal should be ignored if it is undefined
};
} //end Pumpkin namespace