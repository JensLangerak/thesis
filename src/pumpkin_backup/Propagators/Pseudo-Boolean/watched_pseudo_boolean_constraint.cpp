#include "watched_pseudo_boolean_constraint.h"
#include "../../Engine/solver_state.h"

#include <algorithm>
#include <iostream>
#include <set>

namespace Pumpkin
{

WatchedPseudoBooleanConstraint::WatchedPseudoBooleanConstraint(std::vector<BooleanLiteral>& literals, std::vector<uint32_t>& coefficients, uint32_t right_hand_side)
	:right_hand_side_(right_hand_side),
	num_watches_(0)
{
	assert(literals.size() == coefficients.size());
	assert(literals.size() > 0);

	WatchedPseudoBooleanConstraint::ConvertIntoCanonicalForm(coefficients, right_hand_side_);

	for (size_t i = 0; i < literals.size(); i++) { terms_.push_back(Term(literals[i], coefficients[i])); }
	
	//todo: check for overflows
	//todo: who should do the canonical form conversions and normalisations? I suppose we should do it here?
	//todo: who should check for trivial unsat and trivial sat?
	//todo: what about constraints that immediately propagate something? say forcing a literal to be true or false

	//set the watches in order - perhaps there is some better criteria, not sure
	//we would like to ensure that even if any of the watched literal is set to false, the constraint can still be satisfied by the remaining watches

	//add the first literal to watch
	uint32_t sum_of_watches = terms_[0].coefficient;
	num_watches_++;
	uint32_t max_watched_coefficient = terms_[0].coefficient;

	for (size_t i = 1; i < terms_.size(); i++)
	{
		//are we done?
		if (sum_of_watches >= right_hand_side_ + max_watched_coefficient) { break; }

		//not done, add the i-th term to the watch list - todo: need the state to add the watches?
		num_watches_++;
		sum_of_watches += terms_[i].coefficient;
		max_watched_coefficient = std::max(terms_[i].coefficient, max_watched_coefficient);		
	}
	//assert(sum_of_watches - max_watched_coefficient >= right_hand_side_); //otherwise its trivially propagating/unsat, need to handle this case
}

ExplanationPseudoBooleanConstraint * WatchedPseudoBooleanConstraint::ExplainLiteralPropagation(BooleanLiteral literal, SolverState &state) const
{
	return new ExplanationPseudoBooleanConstraint(terms_, right_hand_side_, state, literal);
}

ExplanationPseudoBooleanConstraint * WatchedPseudoBooleanConstraint::ExplainFailure(SolverState &state) const
{
	return new ExplanationPseudoBooleanConstraint(terms_, right_hand_side_, state);
}

Term * WatchedPseudoBooleanConstraint::BeginWatchedTerms()
{
	return &terms_[0];
}

Term * WatchedPseudoBooleanConstraint::EndWatchedTerms()
{
	return &terms_[0] + num_watches_;
}

uint32_t GCD(uint32_t u, uint32_t v) {
	while (v != 0) {
		uint32_t r = u % v;
		u = v;
		v = r;
	}
	return u;
}

void WatchedPseudoBooleanConstraint::ConvertIntoCanonicalForm(std::vector<uint32_t>& coefs, uint32_t &rhs)
{
	//WatchedPseudoBooleanConstraint::Saturate(coefs, rhs);
	//WatchedPseudoBooleanConstraint::Normalise(coefs, rhs);

	//saturate
	for (uint32_t &c : coefs) { c = std::min(c, rhs); }

	if (coefs.size() == 1) { printf("small constraint...\n"); exit(1); }
	//divide by common divisor

	//	find the gcd
	uint32_t div = GCD(coefs[0], coefs[1]);
	for (size_t i = 2; i < coefs.size(); i++) { div = GCD(div, coefs[i]); }

	//	divide
	for (uint32_t &c : coefs) { c /= div; }
	rhs = (rhs / div) + (rhs % div != 0);
}

size_t WatchedPseudoBooleanConstraint::computeLBD(std::vector<Term>& terms, const SolverState & state)
{
	static std::vector<int> seen_decision_level(state.GetNumberOfVariables() + 1, false);

	if (seen_decision_level.size() < state.GetNumberOfVariables() + 1) { seen_decision_level.resize(state.GetNumberOfAssignedVariables() + 1, false); } //+1 since variables are indexed starting from 1
																																						//count the number of unique decision levels by using the seen_decision_level to store whether or not a particular decision level has already been counted
	size_t counter = 0;
	for (Term &term : terms)
	{
		int assignment_level = state.assignments_.GetAssignmentLevel(term.literal.Variable());
		counter += (seen_decision_level[assignment_level] == false);
		seen_decision_level[assignment_level] = true;
	}
	//clear the seen_decision_level data structure - all of its values should be false for the next function call
	for (Term &term : terms)
	{
		int assignment_level = state.assignments_.GetAssignmentLevel(term.literal.Variable());
		seen_decision_level[assignment_level] = false;
	}
	return counter;
}

bool WatchedPseudoBooleanConstraint::IsFailing(SolverState & state)
{
	uint32_t left_hand_side = 0;
	for (Term &term : terms_)
	{
		left_hand_side += (term.coefficient * state.assignments_.IsAssignedTrue(term.literal));
	}
	return left_hand_side < right_hand_side_;
}

bool WatchedPseudoBooleanConstraint::IsCorrect(SolverState & state)
{
	uint32_t max_watched_coef = terms_[0].coefficient;
	for (size_t i = 1; i < num_watches_; i++) { max_watched_coef = std::max(terms_[i].coefficient, max_watched_coef); }

	uint32_t sum_watches = 0;
	for (size_t i = 0; i < num_watches_; i++) { sum_watches += terms_[i].coefficient; }

	if (!(sum_watches - max_watched_coef >= right_hand_side_)) 
	{ 
		return false; 
	}

	for (size_t i = 0; i < num_watches_; i++) 
	{ 
		if (!state.propagator_pseudo_boolean_.constraint_database_.watch_list_.IsLiteralWatchingConstraint(terms_[i].literal, this)) 
		{ 
			return false; 
		}
	}

	//we are done if the constraint is satisfied, no propagation can happen
	//note the constraint might be sat but nonwatches
	uint32_t left_hand_side = 0;
	for (size_t i = 0; i < num_watches_; i++) { left_hand_side += state.assignments_.IsAssignedTrue(terms_[i].literal)*terms_[i].coefficient; }

	if (left_hand_side >= right_hand_side_) { return true; }
	
	//check if there was suppose to be some propagation
	uint32_t sum_nonfalsified_literals = 0;
	for (size_t i = 0; i < terms_.size(); i++) 
	{ 
		sum_nonfalsified_literals += (state.assignments_.IsAssignedFalse(terms_[i].literal) == false)*terms_[i].coefficient;;
	}


	//watches cannot be assigned false unless there is no other option
	uint32_t sum_false_watches = 0;
	std::vector<BooleanLiteral> false_watches;
	for (size_t i = 0; i < num_watches_; i++)
	{
		if (state.assignments_.IsAssignedFalse(terms_[i].literal))
		{
			false_watches.push_back(terms_[i].literal);
			sum_false_watches += terms_[i].coefficient;
		}	
	}

	//false watches are only allowed if there is no other option
	if (sum_false_watches > 0)
	{
		for (size_t i = num_watches_; i < terms_.size(); i++)
		{
			if (state.assignments_.IsAssignedFalse(terms_[i].literal) == false)
			{
				Print(state);
				return false;
			}
		}
	}

	/*if (sum_false_watches > 0)
	{
		bool all_nonwatches_false = true;
		for (size_t i = num_watches_; i < terms_.size(); i++)
		{
			all_nonwatches_false &= state.assignments_.IsAssignedFalse(terms_[i].literal);
		}
		if (all_nonwatches_false == false)
		{
			uint32_t sum_nonfalsified = 0;
			std::cout << "watches\n";
			for (size_t i = 0; i < num_watches_; i++)
			{
				sum_nonfalsified += (state.assignments_.IsAssignedFalse(terms_[i].literal)==false)*terms_[i].coefficient;

				if (state.assignments_.IsAssigned(terms_[i].literal))
				{
					if (terms_[i].literal.IsNegative())
					{
						std::cout << "-";
					}

					std::cout << terms_[i].coefficient << "*";

					std::cout << terms_[i].literal.VariableIndex() << " " << state.assignments_.GetAssignmentLevel(terms_[i].literal.Variable()) << ": " << state.assignments_.IsAssignedTrue(terms_[i].literal) << "\n";
				}
			}
			std::cout << "nowatches: \n";

			for (size_t i = num_watches_; i < terms_.size(); i++)
			{
				sum_nonfalsified += (state.assignments_.IsAssignedFalse(terms_[i].literal)==false)*terms_[i].coefficient;

				if (state.assignments_.IsAssigned(terms_[i].literal))
				{
					std::cout << terms_[i].coefficient << " x ";

					if (terms_[i].literal.IsNegative())
					{
						std::cout << "-";
					}

					std::cout << terms_[i].literal.VariableIndex() << " " << state.assignments_.GetAssignmentLevel(terms_[i].literal.Variable()) << ": " << state.assignments_.IsAssignedTrue(terms_[i].literal) << "\n";
				}
			}

 			return false;
		}
	}*/

	//if (sum_false_watches > max_watched_coef)
	if (sum_false_watches > 0 && (sum_watches - sum_false_watches < right_hand_side_))
	{
		Print(state);
		return false;
	}


	uint32_t slack = sum_nonfalsified_literals - right_hand_side_;
	//propagate every unassigned literal with a coefficients greater than the slack
	for (size_t i = 0; i < terms_.size(); i++)
	{
		if (!state.assignments_.IsAssigned(terms_[i].literal) && slack < terms_[i].coefficient)  //unassigned ones should be set
		{
			Print(state);
			return false; 
		}
	}

	return true;
}

bool WatchedPseudoBooleanConstraint::IsUnsat(SolverState & state, bool all_must_be_assigned)
{
	//we are done if the constraint is satisfied, no propagation can happen
	uint32_t left_hand_side = 0;
	bool all_assigned = true;
	for (size_t i = 0; i < terms_.size(); i++) 
	{ 
		left_hand_side += state.assignments_.IsAssignedTrue(terms_[i].literal)*terms_[i].coefficient;
		all_assigned &= state.assignments_.IsAssigned(terms_[i].literal);
	}

	if (all_must_be_assigned && !all_assigned) { return false; }

	if (all_assigned == false) { return false; }

	return left_hand_side < right_hand_side_;
}

void WatchedPseudoBooleanConstraint::Print(SolverState & state)
{
	uint32_t sum_nonfalsified = 0;
	uint32_t sum_nonfalsified_nonwatches = 0;
	std::cout << "watches\n";
	for (size_t i = 0; i < num_watches_; i++)
	{
		sum_nonfalsified += (state.assignments_.IsAssignedFalse(terms_[i].literal) == false)*terms_[i].coefficient;

		PrintTerm(terms_[i], state);
	}
	std::cout << "nowatches: \n";

	for (size_t i = num_watches_; i < terms_.size(); i++)
	{
		sum_nonfalsified += (state.assignments_.IsAssignedFalse(terms_[i].literal) == false)*terms_[i].coefficient;
		sum_nonfalsified_nonwatches += (state.assignments_.IsAssignedFalse(terms_[i].literal) == false)*terms_[i].coefficient;

		PrintTerm(terms_[i], state);
	}
	std::cout << "Sum nonfalse: " << sum_nonfalsified << std::endl;
	std::cout << "Sum nonfalse nonwatch: " << sum_nonfalsified_nonwatches << std::endl;
}

void WatchedPseudoBooleanConstraint::PrintTerm(Term &term, SolverState & state)
{
	std::cout << term.coefficient << " x ";

	if (term.literal.IsNegative())
	{
		std::cout << "-";
	}

	std::cout << term.literal.VariableIndex();

	if (state.assignments_.IsAssigned(term.literal))
	{
		std::cout << " (" << state.assignments_.GetAssignmentLevel(term.literal.Variable()) << "): " << state.assignments_.IsAssignedTrue(term.literal) << "\n";
	}
	else
	{
		std::cout << ": x\n";
	}
}

bool WatchedPseudoBooleanConstraint::ContainsLiteral(BooleanLiteral literal)
{
	for (Term& term : terms_)
	{
		if (term.literal == literal) { return true; }
	}
	return false;
}

} //end Pumpkin namespace