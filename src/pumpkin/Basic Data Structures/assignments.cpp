#include "assignments.h"
#include "runtime_assert.h"

#include <assert.h>

namespace Pumpkin
{

Assignments::Assignments(size_t number_of_variables)
	:truth_values_(number_of_variables+1, 2), info_(number_of_variables+1) //remember variable indexing goes from 1 and not 0
{
}

bool Assignments::IsAssigned(BooleanVariable variable) const
{
	assert(truth_values_[variable.index_] >= 0 && truth_values_[variable.index_] <= 2);
	return truth_values_[variable.index_] != 2;
}

bool Assignments::IsAssigned(BooleanLiteral literal) const
{
	assert(truth_values_[literal.Variable().index_] >= 0 && truth_values_[literal.Variable().index_] <= 2);
	return truth_values_[literal.Variable().index_] != 2;
}

bool Assignments::IsAssignedTrue(BooleanLiteral literal) const
{	
	assert(truth_values_[literal.Variable().index_] >= 0 && truth_values_[literal.Variable().index_] <= 2);
	return IsAssigned(literal) && truth_values_[literal.Variable().index_] == literal.IsPositive();
}

bool Assignments::IsAssignedTrue(BooleanVariable variable) const
{
	assert(truth_values_[variable.index_] >= 0 && truth_values_[variable.index_] <= 2);
	return truth_values_[variable.index_] == 1;
}

bool Assignments::IsAssignedFalse(BooleanLiteral literal) const
{
	assert(truth_values_[literal.Variable().index_] >= 0 && truth_values_[literal.Variable().index_] <= 2);
	return IsAssigned(literal) && truth_values_[literal.Variable().index_] == literal.IsNegative();
}

bool Assignments::IsPropagatedFalseAtRoot(BooleanLiteral literal) const
{
	return IsAssignedFalse(literal) && IsPropagated(literal.Variable()) && GetAssignmentLevel(literal.Variable()) == 0;
}

bool Assignments::IsPropagatedTrueAtRoot(BooleanLiteral literal) const
{
	return IsAssignedTrue(literal) && IsPropagated(literal.Variable()) && GetAssignmentLevel(literal.Variable()) == 0;
}

bool Assignments::IsDecision(BooleanVariable variable) const
{
	return IsAssigned(variable) && GetAssignmentPropagator(variable) == NULL;
}

bool Assignments::IsPropagated(BooleanVariable variable) const
{
	return IsAssigned(variable) && GetAssignmentPropagator(variable) != NULL;
}

bool Assignments::IsPropagatedAtRoot(BooleanLiteral literal) const
{
	return IsPropagated(literal.Variable()) && GetAssignmentLevel(literal.Variable()) == 0;
}

BooleanLiteral Assignments::GetAssignment(const BooleanVariable variable) const
{
	assert(IsAssigned(variable));
	runtime_assert(variable.index_ < truth_values_.size());
	return BooleanLiteral(variable, truth_values_[variable.index_] == 1);
}

bool Assignments::GetAssignment(BooleanLiteral literal) const
{
	assert(IsAssigned(literal));
	return truth_values_[literal.Variable().index_] == literal.IsPositive();
}

int Assignments::GetAssignmentLevel(BooleanVariable variable) const
{
	assert(IsAssigned(variable));
	return info_[variable.index_].decision_level;
}

uint64_t Assignments::GetAssignmentCode(BooleanVariable variable) const
{
	assert(IsAssigned(variable));
	return info_[variable.index_].code;
}

PropagatorGeneric * Assignments::GetAssignmentPropagator(BooleanVariable variable) const
{
	assert(IsAssigned(variable));
	return info_[variable.index_].responsible_propagator;
}

int Assignments::GetTrailPosition(BooleanVariable variable) const
{
	assert(IsAssigned(variable));
	return info_[variable.index_].position_on_trail;
}

void Assignments::MakeAssignment(const BooleanVariable variable, bool truth_value, int decision_level, PropagatorGeneric *responsible_propagator, uint64_t code, int position_on_trail)
{
	truth_values_[variable.index_] = truth_value;
	assert(truth_values_[variable.index_] == 0 || truth_values_[variable.index_] == 1);
	info_[variable.index_].decision_level = decision_level;
	info_[variable.index_].responsible_propagator = responsible_propagator;
	info_[variable.index_].code = code;
	info_[variable.index_].position_on_trail = position_on_trail;
}

void Assignments::UnassignVariable(BooleanVariable variable)
{
	truth_values_[variable.index_] = 2; //info will not be touched, but we could
}

size_t Assignments::GetNumberOfVariables() const
{
	assert(truth_values_.size() > 0);
	return truth_values_.size() - 1; //remember the 0th index is not used
}

void Assignments::Grow()
{
	truth_values_.push_back(2);
	info_.push_back(AuxiliaryAssignmentInfo());
}

} //end Pumpkin namespace