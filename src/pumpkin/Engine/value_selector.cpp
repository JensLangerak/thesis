#include "value_selector.h"
#include "../Utilities/runtime_assert.h"

namespace Pumpkin
{

ValueSelector::ValueSelector(int num_variables)
	:values_(num_variables, false), //the 0-th position is not used
	is_frozen_(num_variables, false) //the 0-th position is not used
{
}

bool ValueSelector::SelectValue(BooleanVariable variable)
{
	return values_[variable.index_];
}

void ValueSelector::UpdatePolarity(BooleanVariable variable, bool truth_value)
{
	runtime_assert(variable.IsUndefined() == false);

	if (is_frozen_[variable.index_]) return; //no updates if the values are frozen for this variable
	values_[variable.index_] = truth_value;
}

void ValueSelector::Grow()
{
	values_.Grow(false);
	is_frozen_.Grow(false);
}

void ValueSelector::SetPolaritiesToFalse()
{
	for (int i = 1; i <= values_.NumVariables(); i++) { values_[i] = false; }
}

void ValueSelector::FreezeCurrentPhaseValues()
{
	for (int i = 1; i <= is_frozen_.NumVariables(); i++) { is_frozen_[i] = true; }
}

void ValueSelector::UnfreezeAll()
{
	for (int i = 1; i <= is_frozen_.NumVariables(); i++) { is_frozen_[i] = false; }
}

void ValueSelector::SetAndFreezeValue(BooleanLiteral frozen_literal)
{
	values_[frozen_literal.Variable()] = frozen_literal.IsPositive();
	is_frozen_[frozen_literal.Variable()] = true;
}

void ValueSelector::SetPhaseValuesAndFreeze(const BooleanAssignmentVector& solution)
{
	InitialiseValues(solution);
	FreezeCurrentPhaseValues();
}

void ValueSelector::InitialiseValues(const BooleanAssignmentVector& solution)
{
	while (values_.NumVariables() < solution.NumVariables()) { Grow(); }
	for (int i = 1; i <= solution.NumVariables(); i++) { values_[i] = solution[i]; }
	for (int i = solution.NumVariables() + 1; i <= values_.NumVariables(); i++) { values_[i] = false; } //todo, not a great solution but okay for now. The problem is that the input solution may have less variables than the the solver has variables, which happens whenever we add new encodings without updating the previous solution
}

} //end Pumpkin namespace