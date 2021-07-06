#ifndef BOOLEAN_VARIABLE_H
#define BOOLEAN_VARIABLE_H

#include <stdlib.h>

namespace Pumpkin
{

/*
A class representing a Boolean variable. This class is mainly introduced for clarity, e.g. to avoid misusing integers, learned_clause_literals, and variables as one another
*/
class BooleanVariable
{
public:
	BooleanVariable(); // create an undefined variable
	explicit BooleanVariable(long index); //index > 0 must be
	BooleanVariable &operator=(BooleanVariable);

	bool IsUndefined() const; //returns true or false if the variable is considered undefined. Internally a special code_ for undefined variables is kept to distinguish it, i.e. the never-used index zero variable

	bool operator==(BooleanVariable); //compares if two variables are identical
	bool operator!=(BooleanVariable);

	size_t ToPositiveInteger() const; //used for the VectorObjectIndexed

	long index_;
};

} //end Pumpkin namespace

#endif // !BOOLEAN_VARIABLE_H