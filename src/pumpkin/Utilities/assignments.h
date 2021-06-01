#ifndef ASSIGNMENTS_H
#define ASSIGNMENTS_H

#include "boolean_variable.h"
#include "boolean_literal.h"
#include "small_helper_structures.h"

#include <vector>

namespace Pumpkin
{

class PropagatorGeneric;

class Assignments
{
public:
	Assignments(size_t number_of_variables);

	bool IsAssigned(BooleanVariable) const; //reports whether the Boolean variable has been assigned a value (false if undefined, otherwise true)
	bool IsAssigned(BooleanLiteral) const; //report whether the literal has been assigned a value (false if not, other true)
	bool IsAssignedTrue(BooleanLiteral) const; //reports whether the literal has been assigned true. If the literal is unassigned or evaluates to false, the method returns false.
	bool IsAssignedTrue(BooleanVariable) const; //reports whether the variable has been assigned true. If the literal is unassigned or evaluates to false, the method returns false.
	bool IsAssignedFalse(BooleanLiteral) const; //reports whether the literal has been assigned false. If the literal is unassigned or evaluates to true, the method returns false 
	bool IsDecision(BooleanVariable) const;//reports whether the variable has been assigned as a result of a decision.
	bool IsDecision(BooleanLiteral) const;
	bool IsPropagated(BooleanVariable) const;//reports if the variable has been assigned as a result of a propagation rather than a decision
	bool IsPropagatedAtRoot(BooleanLiteral) const;//reports whether the Boolean literal has been propagated to either value at the root level.
	bool IsPropagatedFalseAtRoot(BooleanLiteral) const;//reports whether the Boolean literal has been propagated to false at the root level (decision level = 0). Note that these literals can effectiely be preprocessed out of the instance.
	bool IsPropagatedTrueAtRoot(BooleanLiteral) const;//reports whether the Boolean literal has been propagated to true at the root level (decision level = 0). Note that these literals can effectiely be preprocessed out of the instance.
	bool IsRootAssignment(BooleanLiteral) const; //reports whether the literal has been set at the root (level 0), either by propagation or as a unit clause

	BooleanLiteral GetAssignment(const BooleanVariable) const; //return the literal of the corresponding variable that has been assigned 'true'. Assumes the variable has been assigned a value, i.e. it is not undefined.
	bool GetAssignment(BooleanLiteral) const; //returns the truth value (true, false) assigned to the literal. It assumes the corresponding Boolean variable has been assigned a value, i.e. it is not undefined.
	int  GetAssignmentLevel(BooleanVariable) const; //return the level when the variable was assigned a value. Assumes the variables has been assigned a value. 
	int  GetAssignmentLevel(BooleanLiteral) const; //return the level when the variable was assigned a value. Assumes the variables has been assigned a value.
	uint64_t GetAssignmentCode(BooleanVariable) const;
	PropagatorGeneric * GetAssignmentPropagator(BooleanVariable) const; //returns the clause_ that propagated the assignment of the variable. Assumes the variable has been assigned and, for now, assumes the assignment was implied rather than being a decision
	PropagatorGeneric* GetAssignmentPropagator(BooleanLiteral) const;
	int GetTrailPosition(BooleanVariable) const;

	void MakeAssignment(const BooleanVariable variable, bool truth_value, int decision_level, PropagatorGeneric *responsible_propagator, uint64_t code, int position_on_trail);
	void UnassignVariable(BooleanVariable variable);

	size_t GetNumberOfVariables() const;

	void Grow();

private:
	//TODO consider converting into a bitset later on
	std::vector<char> truth_values_; //[i] is the truth value assignment for the i-th variable with the codes: 2 - unassigned; 1 - true; 0 - false
	std::vector<AuxiliaryAssignmentInfo> info_; //stores additional information about the assignments
};

} //end Pumpkin namespace

#endif // !ASSIGNMENTS_H
