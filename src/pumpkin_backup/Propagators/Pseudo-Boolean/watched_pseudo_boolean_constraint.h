#ifndef WATCHED_PSEUDO_BOOLEAN_CONSTRAINT
#define WATCHED_PSEUDO_BOOLEAN_CONSTRAINT

#include <vector>

#include "../../Basic Data Structures/boolean_literal.h"
#include "../../Basic Data Structures/small_helper_structures.h"
#include "explanation_pseudo_boolean_constraint.h"
#include "../../Basic Data Structures/learned_pseudo_boolean_constraint.h"

namespace Pumpkin
{

class SolverState;

//represents the pseudo Boolean constraints \sum c_i * x_i >= rhs, where c_i > 0 and x_i is the variable that takes a binary value
//note that other constraints, such as <= and those with negative coefficients can be converted into this form
//uses the lazy watch literal scheme to do propagation, as opposed to an eager counting approach

class WatchedPseudoBooleanConstraint
{
public:
	//it seems more natural to provide the input as an array of terms, rather than literals and coefficients passed separately
	WatchedPseudoBooleanConstraint(std::vector<BooleanLiteral> &literals, std::vector<uint32_t> &coefficients, uint32_t right_hand_side);

	ExplanationPseudoBooleanConstraint * ExplainLiteralPropagation(BooleanLiteral literal, SolverState &state) const; //returns the conjunction that forces the assignment of input literal to true. Assumes the input literal is not undefined.
	ExplanationPseudoBooleanConstraint * ExplainFailure(SolverState &state) const; //returns the conjunction that leads to failure

	Term * BeginWatchedTerms();
	Term * EndWatchedTerms();

	std::vector<Term> terms_;
	uint32_t right_hand_side_;
	uint32_t num_watches_; //the number of watched literals can change during search

	static void ConvertIntoCanonicalForm(std::vector<uint32_t> &coefs, uint32_t &rhs);
	//static void Saturate(std::vector<uint32_t> &coefs, uint32_t &rhs);
	//static void Normalise(std::vector<uint32_t> &coefs, uint32_t &rhs);

	static size_t computeLBD(std::vector<Term>& terms, const SolverState & state);

	bool IsFailing(SolverState &state);
	bool IsCorrect(SolverState &state);
	bool IsUnsat(SolverState &state, bool all_must_be_assigned = false);
	void Print(SolverState &state);
	void PrintTerm(Term &term, SolverState &state);
	bool ContainsLiteral(BooleanLiteral literal);
};

} //end Pumpkin namespace

#endif // !WATCHED_PSEUDO_BOOLEAN_CONSTRAINT
