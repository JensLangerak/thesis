#include "variable_selector.h"
#include "solver_state.h"
#include "../Utilities/runtime_assert.h"

namespace Pumpkin
{

VariableSelector::VariableSelector(int num_variables, double decay_factor):
	heap_(num_variables),
	increment_(1.0),
	max_threshold_(1e100),
	decay_factor_(decay_factor),bit_strings_(num_variables)
{
}

double VariableSelector::GetActiviy(BooleanVariable variable) {

  double value = heap_.GetKeyValue(variable.index_ - 1);
  return value;

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
  bit_strings_.Increment(boolean_variable.index_ -1);
}

void VariableSelector::BumpActivity(BooleanVariable variable, double bump_multiplier)
{
	double value = heap_.GetKeyValue(variable.index_ - 1);
	if (value + bump_multiplier *increment_ >= max_threshold_)
	{
		heap_.DivideValues(max_threshold_);
		increment_ /= max_threshold_;
	}
	heap_.Increment(variable.index_ - 1, bump_multiplier *increment_);
  bit_strings_.Increment(variable.index_ -1);
}

void VariableSelector::DecayActivities()
{
	increment_ *= (1.0 / decay_factor_);
  bit_strings_.IncreaseId();
}

BooleanVariable VariableSelector::PeekNextVariable(SolverState * state)
{
	if (heap_.Empty()) { return BooleanVariable(); }

	//make sure the top variable is not assigned
	//iteratively remove top variables until either the heap is empty or the top variable is unassigned
	//note that the data structure is lazy: once a variable is assigned, it may not be removed from the heap, and this is why assigned variable may still be present in the heap
	while (!heap_.Empty() && state->assignments_.IsAssigned(BooleanVariable(heap_.PeekMaxKey() + 1))) 
	{
		heap_.PopMax();
	}

	if (heap_.Empty()) 
	{ 
		return BooleanVariable(); 
	}
	else
	{
		return BooleanVariable(heap_.PeekMaxKey()+1);
	}
}

void VariableSelector::Remove(BooleanVariable boolean_variable)
{
	heap_.Remove(boolean_variable.index_ - 1);
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
  bit_strings_.Grow();
}

bool VariableSelector::IsVariablePresent(BooleanVariable boolean_variable) const
{
	return heap_.IsKeyPresent(boolean_variable.index_-1);
}

void VariableSelector::Reset(int seed)
{
	heap_.Reset(seed);
}

void VariableSelector::RescaleActivities()
{
	heap_.DivideValues(max_threshold_);
}

} //end Pumpkin namespace