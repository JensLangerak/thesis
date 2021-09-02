#include "reason_pseudo_boolean_constraint.h"
#include "watched_pseudo_boolean_constraint.h"
#include "../../Utilities/learned_pseudo_boolean_constraint_iterator.h"
#include "../../Engine/solver_state.h"
#include "../../Utilities/runtime_assert.h"

#include <iostream>
#include <climits>

namespace Pumpkin
{

ReasonPseudoBooleanConstraint::ReasonPseudoBooleanConstraint(WatchedPseudoBooleanConstraint * constraint, SolverState &state) :
	pseudo_boolean_constraint_(state.GetNumberOfVariables())
{
	//remember it is important to first add the right hand side because saturation will take place when adding individual terms
	pseudo_boolean_constraint_.AddToRightHandSide(constraint->right_hand_side_);
	for (const Term &term : constraint->terms_)
	{
		//if the literal is assigned at root level, treat it differently since these literals are effectively not in the instance
		if (state.assignments_.IsAssigned(term.literal) && state.assignments_.GetAssignmentLevel(term.literal.Variable()) == 0)
		{
			//do not add it to the constraint but instead weaken it immediately for true literals
			if (state.assignments_.IsAssignedTrue(term.literal))
			{
				pseudo_boolean_constraint_.RemoveFromRightHandSide(term.coefficient);
			}
			//note that we simply ignore literals that are falsfied at the root since they do not contribute to the problem
		}
		else
		{
			pseudo_boolean_constraint_.AddTerm(term);
		}
	}

	//todo I could remove things that are not necessarily related to the conflict
}

ReasonPseudoBooleanConstraint::ReasonPseudoBooleanConstraint(WatchedPseudoBooleanConstraint * constraint, BooleanLiteral propagated_literal, SolverState & state):
	pseudo_boolean_constraint_(state.GetNumberOfVariables())
{
	runtime_assert(propagated_literal.IsUndefined() || constraint->ContainsLiteral(propagated_literal) || constraint->ContainsLiteral(~propagated_literal));
	//std::cout << propagated_literal.IsUndefined() << constraint->ContainsLiteral(propagated_literal) << constraint->ContainsLiteral(~propagated_literal) << "\n";

	//remember it is important to first add the right hand side because saturation will take place when adding individual terms
	uint32_t propagated_literal_coefficient = 0;
	pseudo_boolean_constraint_.AddToRightHandSide(constraint->right_hand_side_);
	for (const Term &term : constraint->terms_)
	{
		if (term.literal == propagated_literal)
		{
			propagated_literal_coefficient = term.coefficient; //todo remove, only for debugging
			//continue; //we need to add the literal to the reason inequality -> can be avoid that, probably not?
		}

		//if the literal is assigned at root level, treat it differently since these literals are effectively not in the instance
		if (state.assignments_.IsAssigned(term.literal) && state.assignments_.GetAssignmentLevel(term.literal.Variable()) == 0)
		{
			//do not add it to the constraint but instead weaken it immidiately for true literals
			if (state.assignments_.IsAssignedTrue(term.literal))
			{
				pseudo_boolean_constraint_.RemoveFromRightHandSide(term.coefficient);
			}
			//note that we simply ignore literals that are falsfied at the root since they do not contribute to the problem
		}
		else
		{
			pseudo_boolean_constraint_.AddTerm(term);
		}
	}
	runtime_assert(propagated_literal_coefficient > 0); //the literal must be present if it was propagated

	//todo I could integrating this in the loop above
	//todo I could possibly weaken more, e.g. literals that can be removed without changing the propagation, need to think about this
	//	but I am not entirely sure if it is a good idea?
	
	//weaken away literals that appear after the literal on the stack
	int propagated_literal_trail_position = state.assignments_.GetTrailPosition(propagated_literal.Variable());
	for (const Term &term : constraint->terms_)
	{
		int literal_trail_position = state.assignments_.GetTrailPosition(term.literal.Variable());
		if (propagated_literal_trail_position < literal_trail_position)
		{
			pseudo_boolean_constraint_.WeakenLiteral(term.literal);
		}
	}
}

void ReasonPseudoBooleanConstraint::RoundToOne(BooleanLiteral resolving_literal, SolverState & state)
{
	if (pseudo_boolean_constraint_.GetCoefficient(resolving_literal) == 0)
	{
		std::cout << "Size " << pseudo_boolean_constraint_.Size() << "\n";
		std::cout << pseudo_boolean_constraint_.GetCoefficient(resolving_literal) << "\n";
		std::cout << pseudo_boolean_constraint_.GetCoefficient(~resolving_literal) << "\n";
		std::cout << pseudo_boolean_constraint_.GetRightHandSide() << "\n";
	}

	runtime_assert(pseudo_boolean_constraint_.GetCoefficient(resolving_literal) > 0);
	pseudo_boolean_constraint_.RoundToOne(resolving_literal, state);
}

Term ReasonPseudoBooleanConstraint::operator[](size_t index)
{
	//todo this is a hack! Should fix it in the future...
	LearnedPseudoBooleanConstraintIterator iterator = pseudo_boolean_constraint_.GetIterator();
	for (int i = 0; i < index; i++) { ++iterator; }

	BooleanLiteral literal = iterator.Literal();//pseudo_boolean_constraint_.GetJthLiteral(index);
	uint64_t coefficient = iterator.Coefficient();//pseudo_boolean_constraint_.GetCoefficient(literal);
	assert(coefficient <= UINT_MAX);
	return Term(literal, uint32_t(coefficient));
}

uint64_t ReasonPseudoBooleanConstraint::GetCoefficient(BooleanLiteral literal)
{
	return pseudo_boolean_constraint_.GetCoefficient(literal);
}

void ReasonPseudoBooleanConstraint::MultiplyByFraction(uint64_t numerator, uint64_t denominator)
{
	pseudo_boolean_constraint_.MultiplyByFraction(numerator, denominator);
}

uint32_t ReasonPseudoBooleanConstraint::GetRightHandSide()
{
	return pseudo_boolean_constraint_.GetRightHandSide();
}

size_t ReasonPseudoBooleanConstraint::Size()
{
	return pseudo_boolean_constraint_.Size();
}

} //end Pumpkin namespace