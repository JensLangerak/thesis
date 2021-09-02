#pragma once

#include "../Utilities/boolean_literal.h"

namespace Pumpkin
{

//a class representing the explanation of a either a failure or a propagation
//an explanation is a conjunction of literals that leads to either a failure or a propagation

class ExplanationGeneric
{
public:
	virtual BooleanLiteral operator[](int index) const = 0;
	virtual size_t Size() const = 0;
        virtual ~ExplanationGeneric() = default; //adding a private constructor to ensure that only dedicated classes can create/delete explanations
protected:
	ExplanationGeneric() {} //adding a private constructor to ensure that only dedicated classes can create/delete explanations
};

} //end Pumpkin namespace