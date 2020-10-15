#include "explanation_pseudo_boolean_constraint.h"
#include "watched_pseudo_boolean_constraint.h"
#include "../../Engine/solver_state.h"

#include <assert.h>
#include <algorithm>

namespace Pumpkin
{

ExplanationPseudoBooleanConstraint::ExplanationPseudoBooleanConstraint(const std::vector<Term>& terms, uint32_t right_hand_side, SolverState &state)
{
	InitialiseExplanation(terms, right_hand_side, state, BooleanLiteral());
}

ExplanationPseudoBooleanConstraint::ExplanationPseudoBooleanConstraint(const std::vector<Term>& terms, uint32_t right_hand_side, SolverState &state, BooleanLiteral propagated_literal)
{
	InitialiseExplanation(terms, right_hand_side, state, propagated_literal);
}

BooleanLiteral ExplanationPseudoBooleanConstraint::operator[](int index) const
{
	return explanation_[index].literal;
}

size_t ExplanationPseudoBooleanConstraint::Size() const
{
	return explanation_.size();
}

void ExplanationPseudoBooleanConstraint::InitialiseExplanation(const std::vector<Term>& terms, uint32_t right_hand_side, SolverState &state, BooleanLiteral literal_to_ignore)
{
	std::vector<Term> heap_terms_; //reserve? todo

	//only consider terms that have been assigned a value
	uint32_t propagated_literal_coefficient = 0;
	for (const Term &term : terms)
	{
		//I think we need to ignore more things, e.g. all literals that were assigned after the literal to ignore TODO
		if (!literal_to_ignore.IsUndefined() && literal_to_ignore == term.literal) 
		{ 
			propagated_literal_coefficient = term.coefficient; 
			continue; 
		}

		if (state.assignments_.IsAssignedFalse(term.literal)) 
		{ 
			heap_terms_.push_back(term); 
		}
	}
	assert(literal_to_ignore.IsUndefined() || propagated_literal_coefficient > 0);
	assert(right_hand_side > 0);
	std::make_heap(heap_terms_.begin(), heap_terms_.end(), [&state](const Term &t1, const Term &t2) { return state.assignments_.GetTrailPosition(t1.literal.Variable()) > state.assignments_.GetTrailPosition(t2.literal.Variable()); });

	int slack = 0;
	for (const Term &term : terms)
	{
		slack += term.coefficient;
	}
	assert(slack >= right_hand_side);
	assert(slack >= propagated_literal_coefficient);
	slack -= right_hand_side;

	int num_elements_inspected = 0;
	//note that for failures, the coefficient will be zero
	//todo do I need this >= 0? The coef for conflict is zero...so...?
	while (slack >= 0 && slack >= propagated_literal_coefficient)  //todo unsigned + signed comparison problem
	{
		assert(num_elements_inspected < heap_terms_.size());

		std::pop_heap(heap_terms_.begin(), heap_terms_.begin() + (heap_terms_.size() - num_elements_inspected), [&state](const Term &t1, const Term &t2) { return state.assignments_.GetTrailPosition(t1.literal.Variable()) > state.assignments_.GetTrailPosition(t2.literal.Variable()); });
		num_elements_inspected++;

		Term candidate_term = heap_terms_[heap_terms_.size() - num_elements_inspected];
		assert(state.assignments_.IsAssignedFalse(candidate_term.literal));
		assert(explanation_.size() == 0 || state.assignments_.GetTrailPosition(explanation_.back().literal.Variable()) < state.assignments_.GetTrailPosition(candidate_term.literal.Variable()));

		slack -= heap_terms_[heap_terms_.size() - num_elements_inspected].coefficient;
		candidate_term.literal = ~candidate_term.literal;
		explanation_.push_back(candidate_term);

		assert(literal_to_ignore.IsUndefined() || state.assignments_.GetTrailPosition(literal_to_ignore.Variable()) > state.assignments_.GetTrailPosition(candidate_term.literal.Variable()));
	}
	//assert(num_elements_inspected < heap_terms_.size()); //cannot inspect all elements otherwise the constraint is trivial?
	
	//try to minimise the terms as a postprocessing step by removing the terms with the smallest coefficient if possible
	num_elements_inspected = 0;
	std::make_heap(explanation_.begin(), explanation_.end(), [&state](const Term &t1, const Term &t2) { return t1.coefficient > t2.coefficient; });
	std::pop_heap(explanation_.begin(), explanation_.begin() + (explanation_.size() - num_elements_inspected), [&state](const Term &t1, const Term &t2) { return t1.coefficient > t2.coefficient; });
	num_elements_inspected = 1;
	Term candidate_term = explanation_[explanation_.size() - num_elements_inspected];
	
	while (slack + candidate_term.coefficient < propagated_literal_coefficient)
	{		
		slack += candidate_term.coefficient;
		num_elements_inspected++;
		std::pop_heap(explanation_.begin(), explanation_.begin() + (explanation_.size() - num_elements_inspected), [&state](const Term &t1, const Term &t2) { return t1.coefficient > t2.coefficient; });
		candidate_term = explanation_[explanation_.size() - num_elements_inspected];
	}
	num_elements_inspected--; //the last inspected should be kept in the explanation
	assert(explanation_.back().coefficient <= explanation_.front().coefficient); //the one at the back is the smallest, should be smaller than a randomly selected element
	explanation_.resize(explanation_.size() - num_elements_inspected); //the element at the back are the ones with the smallest coefficients
}

} //end Pumpkin namespace