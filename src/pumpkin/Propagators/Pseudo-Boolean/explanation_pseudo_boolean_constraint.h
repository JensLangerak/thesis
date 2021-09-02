#ifndef EXPLANATION_PSEUDO_BOOLEAN_CONSTRAINT_H
#define EXPLANATION_PSEUDO_BOOLEAN_CONSTRAINT_H

#include "../explanation_generic.h"

#include <vector>

namespace Pumpkin
{

struct Term;
class SolverState;

class ExplanationPseudoBooleanConstraint : public ExplanationGeneric
{
public:
	ExplanationPseudoBooleanConstraint(const std::vector<Term> &terms, uint32_t right_hand_side, SolverState &state);
	ExplanationPseudoBooleanConstraint(const std::vector<Term> &terms, uint32_t right_hand_side, SolverState &state, BooleanLiteral propagated_literal);

	virtual BooleanLiteral operator[](int index) const;
	virtual size_t Size() const;
private:
	
	//std::vector<Term> heap_terms_; //the value for each term is its position in the stack, with lower values being better
	std::vector<Term> explanation_;

	void InitialiseExplanation(const std::vector<Term> &terms, uint32_t right_hand_side, SolverState &state, BooleanLiteral literal);
};

} //end Pumpkin namespace

#endif // !EXPLANATION_PSEUDO_BOOLEAN_CONSTRAINT_H
