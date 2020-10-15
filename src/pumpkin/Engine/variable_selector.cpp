#include "variable_selector.h"

namespace Pumpkin
{

VariableSelector::VariableSelector(int num_variables)
	:heap_(num_variables),increment_(1.0),max_threshold_(1e100),decay_factor_(0.95)
{
}

void VariableSelector::BumpActivity(BooleanVariable boolean_variable)
{
	double value = heap_.GetKeyValue(boolean_variable.index_ - 1);
	if (value + increment_ >= max_threshold_)
	{
		heap_.DivideValues(max_threshold_);
		increment_ /= max_threshold_;
	}
	heap_.Increment(boolean_variable.index_-1, increment_);
}

void VariableSelector::DecayActivities()
{
	increment_ *= (1.0 / decay_factor_);
}

BooleanVariable VariableSelector::PopHighestActivityVariable()
{
	if (heap_.Size() > 0)
	{
		return BooleanVariable(heap_.PopMax() + 1);
	}
	else
	{
		return BooleanVariable();
	}
	
}

void VariableSelector::Remove(BooleanVariable boolean_variable)
{
	heap_.Remove(boolean_variable.index_-1);
}

void VariableSelector::Readd(BooleanVariable boolean_variable)
{
	if (heap_.IsKeyPresent(boolean_variable.index_-1) == false)
	{
		heap_.Readd(boolean_variable.index_ - 1);
	}	
}


int VariableSelector::Size() const
{
	return heap_.Size();
}

void VariableSelector::Grow()
{
	heap_.Grow();
}

bool VariableSelector::IsVariablePresent(BooleanVariable boolean_variable) const
{
	return heap_.IsKeyPresent(boolean_variable.index_-1);
}

void VariableSelector::RescaleActivities()
{
	heap_.DivideValues(max_threshold_);
}

} //end Pumpkin namespace