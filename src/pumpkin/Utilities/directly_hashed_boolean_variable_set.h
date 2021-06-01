#pragma once

#include "boolean_literal.h"
#include "vector_object_indexed.h"

#include <vector>

namespace Pumpkin
{
//todo description

class DirectlyHashedBooleanVariableSet
{
public:
	void Insert(BooleanVariable);
	void Remove(BooleanVariable);
	void Clear();

	bool IsPresent(BooleanVariable) const;
	int NumPresentValues() const;
	bool Empty() const;

	typename std::vector<BooleanVariable>::const_iterator begin() const;
	typename std::vector<BooleanVariable>::const_iterator end() const;
private:
	void Grow(BooleanVariable); //grows internal data structures to accommodate for the new literal if necessary
	bool IsLiteralOutOfBounds(BooleanVariable) const; //returns true if the literal cannot be inserted without growing the internal data structures

	std::vector<BooleanVariable> present_variables_;
	VectorObjectIndexed<BooleanVariable, int> variable_location_;
};
}//end namespace Pumpkin