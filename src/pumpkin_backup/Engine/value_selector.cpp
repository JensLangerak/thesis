#include "value_selector.h"

#include <assert.h>

namespace Pumpkin
{

ValueSelector::ValueSelector(int num_variables)
	:values_(num_variables+1, false), //the 0-th position is not used
	are_values_predetermined_(false),
	is_frozen_(num_variables+1, false) //the 0-th position is not used
{
}

bool ValueSelector::SelectValue(BooleanVariable variable)
{
	return values_[variable.index_];
}

void ValueSelector::UpdatePolarity(BooleanVariable variable, bool truth_value, bool force)
{
	if ((!force) && (are_values_predetermined_ || is_frozen_[variable.index_])) return; //no updates if the values are forced

	assert(variable.IsUndefined() == false);
	values_[variable.index_] = truth_value;
}

void ValueSelector::Grow()
{
	values_.push_back(false);
	is_frozen_.push_back(false);
}

void ValueSelector::SetPolaritiesToFalse()
{
	for (int i = 0; i < values_.size(); i++) { values_[i] = false; }
}

void ValueSelector::UnfreezeAll()
{
	for (int i = 0; i < is_frozen_.size(); i++) { is_frozen_[i] = false; }
}

void ValueSelector::SetAndFreezeValue(BooleanLiteral frozen_literal)
{
	printf("not used\n");
	exit(1);
	values_[frozen_literal.VariableIndex()] = frozen_literal.IsPositive();
	is_frozen_[frozen_literal.VariableIndex()] = true;

}

void ValueSelector::PredetermineValues(std::vector<bool>& solution)
{
	InitialiseValues(solution);
	are_values_predetermined_ = true;
}

void ValueSelector::InitialiseValues(std::vector<bool>& solution)
{
	assert(values_.size() == solution.size());
	values_ = solution;
}

} //end Pumpkin namespace