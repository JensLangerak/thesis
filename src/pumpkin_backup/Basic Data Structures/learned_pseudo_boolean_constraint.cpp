#include "learned_pseudo_boolean_constraint.h"
#include "learned_pseudo_boolean_constraint_iterator.h"
#include "../Engine/solver_state.h"
#include "fraction_unsigned_64bit.h"
#include "runtime_assert.h"

#include <algorithm>
#include <iostream>
#include <climits>

namespace Pumpkin
{

LearnedPseudoBooleanConstraint::LearnedPseudoBooleanConstraint(size_t num_variables) :
	present_literals_(0), //remember +1 and +2 because index of variables goes from zero
	variable_location_(num_variables+1, -1), 
	map_literal_coefficient64_(num_variables * 2 + 2),
	right_hand_side64_(0)
{
}

LearnedPseudoBooleanConstraint::LearnedPseudoBooleanConstraint(const LearnedPseudoBooleanConstraint &source_constraint):
	present_literals_(source_constraint.present_literals_),
	variable_location_(source_constraint.variable_location_),
	map_literal_coefficient64_(source_constraint.map_literal_coefficient64_),
	right_hand_side64_(source_constraint.right_hand_side64_)
{
}

void LearnedPseudoBooleanConstraint::RoundToOne(BooleanLiteral rounding_literal, SolverState &state)
{
	assert(CheckCorrectness(rounding_literal));
	runtime_assert(GetCoefficient(rounding_literal) > 0); //must be present in the constraint
	runtime_assert(right_hand_side64_ > 0);
	
	//todo check: if weakening triggers saturation, is this a problem? Fixed with using getcoefficient?
	uint64_t original_coefficient = GetCoefficient(rounding_literal);
	for (BooleanLiteral present_literal : present_literals_)
	{
		assert(CheckCorrectness(present_literal));
		//do we need to weaken things that did not contribute to the conflict? Say literals that were placed after the propagated literal?
		bool could_weaken = state.assignments_.IsAssignedFalse(present_literal) == false || (state.assignments_.GetTrailPosition(present_literal.Variable()) > state.assignments_.GetTrailPosition(rounding_literal.Variable()));
		bool not_divisible = (GetCoefficient(present_literal) % original_coefficient) != 0;

		if (could_weaken && not_divisible)
		{ 
			runtime_assert(present_literal.Variable() != rounding_literal.Variable());
			WeakenLiteral(present_literal);
		}
	}

	for (BooleanLiteral present_literal : present_literals_)
	{
		map_literal_coefficient64_[present_literal] = DivideRoundUp(map_literal_coefficient64_[present_literal], original_coefficient);
	}	

	right_hand_side64_ = DivideRoundUp(right_hand_side64_, original_coefficient);// GetCoefficient(rounding_literal));
	runtime_assert(!CheckOverflow());	
	runtime_assert(GetCoefficient(rounding_literal) == 1); //must be present in the constraint
}

void LearnedPseudoBooleanConstraint::AddTerm(const Term &term)
{
	assert(term.coefficient > 0);
	assert(CheckCorrectness(term.literal));

	//first time we see the corresponding variable, initialise
	if (IsVariablePresent(term.Variable()) == false)
	{
		map_literal_coefficient64_[term.literal] = term.coefficient;
		variable_location_[term.Variable()] = present_literals_.size(); //note that this line must happen before the push back in the next one
		present_literals_.push_back(term.literal);
	}
	//easy case when the polarities match (the negative present_literal has a zero coefficient), just add them up
	else if (GetCoefficient(~term.literal) == 0)
	{
		map_literal_coefficient64_[term.literal] += term.coefficient;
	}
	//more involved case when the polarities do not match
	else
	{
		if (GetCoefficient(~term.literal) > term.coefficient) //the coefficient of the negative present_literal is greater, so we only need to reduce its value
		{
			right_hand_side64_ -= term.coefficient;
			map_literal_coefficient64_[~term.literal] -= term.coefficient; //due to the IF condition this coefficient always ends up positive
		}
		else if (GetCoefficient(~term.literal) < term.coefficient) //the present_literal in the constraint will now change sign
		{
			right_hand_side64_ -= GetCoefficient(~term.literal);
			map_literal_coefficient64_[term.literal] = (term.coefficient - GetCoefficient(~term.literal));
			map_literal_coefficient64_[~term.literal] = 0;

			int location = variable_location_[term.Variable()];
			runtime_assert(present_literals_[location] == ~term.literal);
			present_literals_[location] = term.literal;
		}
		else //the present_literal is cancelled out, i.e., GetCoefficient(~present_literal) == term.coefficient
		{
			right_hand_side64_ -= term.coefficient;
			map_literal_coefficient64_[~term.literal] = 0;
			RemoveZeroCoefficientVariableFromDataStructures(term.Variable());
		}
	}
}

void LearnedPseudoBooleanConstraint::AddToRightHandSide(uint32_t val)
{
	right_hand_side64_ += val;
}

void LearnedPseudoBooleanConstraint::RemoveFromRightHandSide(uint32_t val)
{
	runtime_assert(right_hand_side64_ > val); //I am not sure but I think we should never get trivial constraints
	right_hand_side64_ -= val;
}

void LearnedPseudoBooleanConstraint::MultiplyByFraction(uint64_t numerator, uint64_t denominator)
{
	//need to be careful, since GetCoefficient automatically does saturation, which might not be desirable during the process
	//before multiplying, we first need to make sure saturation is in place (perhaps this can be avoided in future versions)
	//...I am not entirely sure if it is necessary, though, need to check. If saturation should take took place before the multiplication,
	//then it will also need to take place after multiplication. So, I think it is not necessary to saturate first, since the end result
	//should be the same independent of whether we saturated before or not. 
	//but for now I'm saturating just to be sure, and later I will remove it

	Saturate();

	FractionUnsigned64Bit multiplier(numerator, denominator);

	FractionUnsigned64Bit new_right_hand_side = multiplier * right_hand_side64_;
	assert(new_right_hand_side.IsInteger());
	right_hand_side64_ = new_right_hand_side.RoundDownToInteger();
	for (BooleanLiteral literal : present_literals_)
	{
		uint64_t coef = map_literal_coefficient64_[literal];
		FractionUnsigned64Bit new_coefficient = multiplier * coef;
		assert(new_coefficient.IsInteger());
		map_literal_coefficient64_[literal] = new_coefficient.RoundDownToInteger();
	}
}

bool LearnedPseudoBooleanConstraint::IsVariablePresent(BooleanVariable var)
{
	assert(variable_location_[var] == -1 || present_literals_[variable_location_[var]].Variable() == var);
	return variable_location_[var] >= 0;
}

void LearnedPseudoBooleanConstraint::RemoveZeroCoefficients()
{
	assert(1 == 2); //I think I should never need to make this call?

	size_t current_index = 0, new_end_index = 0;
	while (current_index < present_literals_.size())
	{
		BooleanLiteral literal = present_literals_[current_index];
		if (GetCoefficient(literal) == 0)
		{
			variable_location_[literal.Variable()] = -1;
		}		
		else
		{
			present_literals_[new_end_index] = present_literals_[current_index];
			new_end_index++;
		}
		current_index++;
	}
	present_literals_.resize(new_end_index);
}

bool LearnedPseudoBooleanConstraint::CheckZeroCoefficients()
{
	for (BooleanLiteral literal : present_literals_) { assert(GetCoefficient(literal) > 0); }
	return true;
}

LearnedPseudoBooleanConstraintIterator LearnedPseudoBooleanConstraint::GetIterator()
{
	return LearnedPseudoBooleanConstraintIterator(*this);
}

bool LearnedPseudoBooleanConstraint::IsClause() const
{
	return GetRightHandSide() == 1;
}

//todo (almost) copy pasted code, need to unify - I think I will introduce iterators which will make unification possible
//also this method does not fit the class, e.g. it would be more appropriate if the propagator class had to figure out the propagation, perhaps later when I introduce iterators for constraints

/*PairUnsatFlagPropagationLiterals LearnedPseudoBooleanConstraint::GetPropagationsFromDecisionLevel(int decision_level, SolverState &state)
{
	assert(state.GetCurrentDecisionLevel() >= decision_level);
	uint32_t lhs = 0;
	for (size_t i = 0; i < Size(); i++)
	{
		BooleanLiteral present_literal = GetJthLiteral(i);
		
		if (GetCoefficient(present_literal) == 0) { continue; }

		int literal_decision_level = state.assignments_.GetAssignmentLevel(present_literal.Variable());
		
		//for literals assigned at decision level or lower, we only add their contribution to lhs if they are unassigned or assigned to zero
		if (literal_decision_level <= decision_level)
		{
			lhs += GetCoefficient(present_literal)*(state.assignments_.IsAssignedFalse(present_literal) == false);
		}
		else //the present_literal is assigned after the given decision level, then take its contribution
		{
			lhs += GetCoefficient(present_literal);
		}		
	}

	//what do we do in this case? return the undefined present_literal?
	//TODO as a hack I will propagate one of the literals to true, and leave it to the propagation engine to figure out that it's unsat
	//the constraint is violated beyond any hope, rip
	if (lhs < GetRightHandSide())
	{
		return PairUnsatFlagPropagationLiterals(true, std::vector<BooleanLiteral>());
	}
	else
	{
		assert(GetRightHandSide() <= lhs); //to be sure overflow will not happen in the next line
		uint32_t slack = lhs - GetRightHandSide();
		//propagate every unassigned present_literal with a coefficients greater than the slack
		for (size_t i = 0; i < Size(); i++)
		{
			BooleanLiteral present_literal = GetJthLiteral(i);
			if (slack < GetCoefficient(present_literal)) //assigning this present_literal to false would break the constraint, hence propagate to true
			{
				if (state.assignments_.IsAssigned(present_literal)) { continue; } //ignore already assigned ones

				uint64_t code = reinterpret_cast<uint64_t>(constraint);
				bool success = state.AddUnitClauseDuringSearch(constraint->terms_[i].present_literal); //todo consider literals that were propagated together?
				if (success == false) { return false; }
			}
		}
		return true;
	}
	return std::vector<BooleanLiteral>();
}*/

size_t LearnedPseudoBooleanConstraint::ComputeLBD(const SolverState & state) const
{
	static std::vector<int> seen_decision_level(state.GetNumberOfVariables() + 1, false);

	if (seen_decision_level.size() < state.GetNumberOfVariables() + 1) { seen_decision_level.resize(state.GetNumberOfAssignedVariables() + 1, false); } //+1 since variables are indexed starting from 1
																																						//count the number of unique decision levels by using the seen_decision_level to store whether or not a particular decision level has already been counted
	size_t counter = 0;
	for (BooleanLiteral literal : present_literals_)
	{
		int assignment_level = state.assignments_.GetAssignmentLevel(literal.Variable());
		counter += (seen_decision_level[assignment_level] == false);
		seen_decision_level[assignment_level] = true;
	}
	//clear the seen_decision_level data structure - all of its values should be false for the next function call
	for (BooleanLiteral literal : present_literals_)
	{
		int assignment_level = state.assignments_.GetAssignmentLevel(literal.Variable());
		seen_decision_level[assignment_level] = false;
	}
	return counter;
}

uint64_t LearnedPseudoBooleanConstraint::GetCoefficient(BooleanLiteral literal)
{
	runtime_assert(literal.IsUndefined() == false);
	assert(CheckCorrectness(literal));
	SaturateCoefficient(literal); //saturation is done lazily since AddTerm can reduce the right hand side
	return map_literal_coefficient64_[literal];
}

std::vector<BooleanLiteral> LearnedPseudoBooleanConstraint::GetLiterals() const
{
	return present_literals_;
}

/*BooleanLiteral LearnedPseudoBooleanConstraint::GetJthLiteral(size_t index) const
{
	return present_literals_[index];
}*/

int64_t LearnedPseudoBooleanConstraint::GetRightHandSide() const
{
	return right_hand_side64_;
}

void LearnedPseudoBooleanConstraint::Grow()
{
	assert(Empty());//I am not sure if this check if necessary - do we add variables during the algorithm? Not at the moment...
	map_literal_coefficient64_.push_back(0); //two new entries, one for each polarity
	map_literal_coefficient64_.push_back(0);
	variable_location_.push_back(-1);
}

void LearnedPseudoBooleanConstraint::Clear()
{
	for (BooleanLiteral literal : present_literals_)
	{
		assert(CheckCorrectness(literal));
		variable_location_[literal.Variable()] = -1;
		map_literal_coefficient64_[literal] = 0;
	}
	present_literals_.clear();
	right_hand_side64_ = 0;
	assert(CheckClear());
}

bool LearnedPseudoBooleanConstraint::Empty() const
{
	return present_literals_.empty();
}

size_t LearnedPseudoBooleanConstraint::Size() const
{
	return present_literals_.size();
}

void LearnedPseudoBooleanConstraint::Saturate()
{
	for (BooleanLiteral literal : present_literals_) { SaturateCoefficient(literal); }
}

void LearnedPseudoBooleanConstraint::SaturateCoefficient(BooleanLiteral literal)
{
	runtime_assert(right_hand_side64_ > 0); //I think we never have these trivial constraints with a zero right hand side
	map_literal_coefficient64_[literal] = std::min(map_literal_coefficient64_[literal], right_hand_side64_);
}

void LearnedPseudoBooleanConstraint::WeakenLiteral(BooleanLiteral literal)
{
	assert(CheckCorrectness(literal));
	assert(right_hand_side64_ > GetCoefficient(literal)); //can we get trivial constraints in our procedure?

	right_hand_side64_ -= GetCoefficient(literal);
	runtime_assert(right_hand_side64_ > 0);
	map_literal_coefficient64_[literal] = 0;
	RemoveZeroCoefficientVariableFromDataStructures(literal.Variable());
}

uint64_t LearnedPseudoBooleanConstraint::DivideRoundUp(uint64_t n, uint64_t divisor)
{
	runtime_assert(n > 0 && divisor > 0);
	return (n / divisor) + ((n % divisor) != 0);
}

bool LearnedPseudoBooleanConstraint::CheckOverflow()
{
	if (right_hand_side64_ >= UINT_MAX) { return true; }
	for (BooleanLiteral literal : present_literals_) 
	{ 
		if (GetCoefficient(literal) > UINT_MAX) { return true; }
	}
	return false;
}

void LearnedPseudoBooleanConstraint::Print()
{
	for (BooleanLiteral literal : present_literals_)
	{
		std::cout << map_literal_coefficient64_[literal] << "x" << literal.ToPositiveInteger() << " + ";
	}
	std::cout << ">= " << right_hand_side64_ << "\n";
}

std::string LearnedPseudoBooleanConstraint::ToString()
{
	std::string s;
	
	for (BooleanLiteral literal : present_literals_)
	{
		assert(CheckCorrectness(literal));
		s += literal.ToString();
		s += " + ";
	}
	s += " >= ";
	s += std::to_string(GetRightHandSide());
	return s;
}

bool LearnedPseudoBooleanConstraint::CheckClear()
{
	for (uint64_t coef : map_literal_coefficient64_) { assert(coef == 0); }
	assert(present_literals_.empty());
	assert(right_hand_side64_ == 0);
	return true;
}

bool LearnedPseudoBooleanConstraint::CheckCorrectness(BooleanLiteral literal) const
{
	BooleanVariable variable = literal.Variable();
	uint64_t coef_pos = map_literal_coefficient64_[literal];
	uint64_t coef_neg = map_literal_coefficient64_[~literal];
	int64_t variable_loc = variable_location_[variable];

	//one of the polarities must have a zero coefficient at all times
	assert((coef_pos == 0) + (coef_neg == 0) >= 1);	
	assert(variable_loc >= 0 || (coef_pos == 0 && coef_neg == 0));//if the variable is not present, its coefficients must be zero.
	assert(variable_loc == -1 || (coef_pos != 0 || coef_neg != 0));//conversely, if the coefficients are zero, then the variable must be absent
	assert(variable_loc == -1 || present_literals_[variable_loc].Variable() == variable);//check that the internal data structure correctly stores the location of the variable
	assert(right_hand_side64_ > 0); //should I allow trivial constraints?
	return true;
}


//the problem here is that if we are iterating through the literals of this constraint
//and say we decide to reduce one of them to zero, e.g. if they are accessed through the j-th present_literal, well the j-th present_literal might change as we nullify some of the coefficients
//the indicies will change! This can be problematic...
void LearnedPseudoBooleanConstraint::RemoveZeroCoefficientVariableFromDataStructures(BooleanVariable zero_variable)
{
	runtime_assert(map_literal_coefficient64_[BooleanLiteral(zero_variable, false)] == 0 && map_literal_coefficient64_[BooleanLiteral(zero_variable, true)] == 0);
	runtime_assert(variable_location_[zero_variable] >= 0);

	BooleanLiteral former_last_literal = present_literals_.back();
	if (former_last_literal.Variable() != zero_variable) 
	{ 
		size_t new_loc = variable_location_[zero_variable];
		variable_location_[former_last_literal.Variable()] = new_loc;
		present_literals_[new_loc] = former_last_literal;
	}	
	variable_location_[zero_variable] = -1;
	present_literals_.pop_back();
}

} //end Pumpkin namespace