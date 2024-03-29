#include "directly_hashed_boolean_variable_set.h"
#include "runtime_assert.h"

namespace Pumpkin
{
void DirectlyHashedBooleanVariableSet::Insert(BooleanVariable var)
{
	runtime_assert(!IsPresent(var)); //for now we are assuming that we will never add a literal that is already present
	Grow(var);
	variable_location_[var] = present_variables_.size();
	present_variables_.push_back(var);
}

void DirectlyHashedBooleanVariableSet::Remove(BooleanVariable var)
{
	runtime_assert(IsPresent(var));
	//we move the last variable in the 'present_variables' vector to the position of the removed literal
	//and then decrease the size of the vector by one
	variable_location_[present_variables_.back()] = variable_location_[var];
	present_variables_[variable_location_[var]] = present_variables_.back();
	present_variables_.pop_back();
	variable_location_[var] = -1;
}

void DirectlyHashedBooleanVariableSet::Clear()
{
	for (BooleanVariable variable : present_variables_) { variable_location_[variable] = -1; }
	present_variables_.clear();
}

bool DirectlyHashedBooleanVariableSet::IsPresent(BooleanVariable var) const
{
	return !IsLiteralOutOfBounds(var) && variable_location_[var] != -1;
}

int DirectlyHashedBooleanVariableSet::NumPresentValues() const
{
	return int(present_variables_.size());
}

bool DirectlyHashedBooleanVariableSet::Empty() const
{
	return NumPresentValues() == 0;
}

typename std::vector<BooleanVariable>::const_iterator DirectlyHashedBooleanVariableSet::begin() const
{
	return present_variables_.begin();
}

typename std::vector<BooleanVariable>::const_iterator DirectlyHashedBooleanVariableSet::end() const
{
	return present_variables_.end();
}

void DirectlyHashedBooleanVariableSet::Grow(BooleanVariable var)
{
	if (var.index_ + 1 > variable_location_.Size()) { variable_location_.Resize(size_t(var.index_ + 1), -1); }
}

bool DirectlyHashedBooleanVariableSet::IsLiteralOutOfBounds(BooleanVariable var) const
{
	return var.index_ >= variable_location_.Size();
}
}
