#pragma once

#include "../Utilities/boolean_variable.h"
#include "vector_object_indexed.h"

#include <vector>

namespace Pumpkin{

class DirectlyHashedBooleanVariableLabeller
{
public:
	DirectlyHashedBooleanVariableLabeller(int size); //values in the set must be within [0,.., size). Remember that usually the zero-th index will not be used.

	void AssignLabel(BooleanVariable variable, char label);

	void Grow();
	void Resize(int new_size);
	void Clear();

	char GetLabel(BooleanVariable) const;

	bool IsLabelled(BooleanVariable) const;
	bool IsAssignedSpecificLabel(BooleanVariable variable, char label); //returns true if the variable has been assigned 'label', and false otherwise. Note that the method returns false for all unlabelled variables.
	
	int GetNumLabelledVariables() const;
	int GetCapacity() const;

private:
	std::vector<BooleanVariable> labelled_variables_;
	VectorObjectIndexed<BooleanVariable, char> variable_labels_;

	const char UNLABELLED;
};
}