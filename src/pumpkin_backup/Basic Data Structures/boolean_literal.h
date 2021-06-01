#ifndef BOOLEAN_LITERAL_H
#define BOOLEAN_LITERAL_H

#include "boolean_variable.h"

#include <string>

namespace Pumpkin
{

/*
A class representing a Boolean literal. This class is mainly introduced for clarity and simplicity.
*/
class BooleanLiteral 
{
public:
	BooleanLiteral(); //create an undefined literal. Only the (in)equality operators can be used with this literal.
	BooleanLiteral(BooleanVariable, bool is_positive); //create a positive or negative literal of the provided Boolean variable

	bool operator==(BooleanLiteral) const; //compares if two literals are equal, i.e. if they belong to the same variable and have the same sign (positive or negative)
	bool operator!=(BooleanLiteral) const; //compares if the two literals are different. This is equivalent to !(operator==)
	bool IsPositive() const; //true if literal is a positive literal of its Boolean variable
	bool IsNegative() const; //true if literal is a negative literal of its Boolean variable
	bool IsUndefined() const; //returns true or false if the literal is considered undefined. Internally a special code_ for undefined literals is kept to distinguish it, i.e. its corresponding Boolean variable is the never-used index zero variable
	BooleanVariable Variable() const; //returns the Boolean variable associated with this literal
	int VariableIndex() const;
	BooleanLiteral operator~() const; //returns a negated version of the literal, e.g. if 'lit' was positive, ~lit is a negative literal of the same Boolean variable

	int ToPositiveInteger() const;
	std::string ToString() const;
//private:
	int code_;
};

} //end Pumpkin namespace

#endif // !BOOLEAN_LITERAL_H