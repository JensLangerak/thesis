#ifndef VALUE_SELECTOR_H
#define VALUE_SELECTOR_H

#include "../Utilities/boolean_literal.h"
#include "../Utilities/boolean_assignment_vector.h"

#include <vector>

namespace Pumpkin
{

class ValueSelector
{
public:
	ValueSelector(int num_variables);
	bool SelectValue(BooleanVariable); //returns the suggested truth value assigned for the given variable
	void UpdatePolarity(BooleanVariable, bool);//sets the polarity of the input variable to the given truth value. As a result, SelectValue will now suggest the input truth value for the variable
	void Grow(); //increases the number of variables considered by one.

	void SetPolaritiesToFalse();
	void FreezeCurrentPhaseValues();
	void UnfreezeAll();

	void SetAndFreezeValue(BooleanLiteral);
	void SetPhaseValuesAndFreeze(const BooleanAssignmentVector& solution);
	void InitialiseValues(const BooleanAssignmentVector& solution);

private:
	BooleanAssignmentVector values_, is_frozen_;
};

} //end Pumpkin namespace

#endif // !VALUE_SELECTOR_H