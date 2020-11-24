#ifndef EXPLANATION_GENERIC_H
#define EXPLANATION_GENERIC_H

#include "../Basic Data Structures/boolean_literal.h"

namespace Pumpkin
{

//a class representing the explanation of a either a failure or a propagation
//an explanation is a conjunction of literals that leads to either a failure or a propagation

class ExplanationGeneric
{
public:
	virtual BooleanLiteral operator[](int index) const = 0;
	virtual size_t Size() const = 0;
        virtual ~ExplanationGeneric() {};
};

} //end Pumpkin namespace

#endif // !EXPLANATION_GENERIC_H