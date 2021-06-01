#pragma once

#include "solver_state.h"
#include "conflict_analysis_result_clausal.h"
#include "../Utilities/directly_hashed_integer_set.h"
#include "../Utilities/directly_hashed_boolean_variable_labeller.h"

namespace Pumpkin
{
class LearnedClauseMinimiser
{
public:
	LearnedClauseMinimiser(SolverState& state);

	//assumes the last literal is the propagating literal
	//removes literals that are dominated in the implication graph from the learned clause. A literal is dominated if a subset of the other literals in the learned clause imply that literal, making the dominated literal redundant.
	//the propagating literal cannot be removed
	//the technique used is described in the following paper:
	//Sörensson, Niklas, and Armin Biere. "Minimizing learned clauses." International Conference on Theory and Applications of Satisfiability Testing. Springer, Berlin, Heidelberg, 2009.
	void RemoveImplicationGraphDominatedLiterals(ConflictAnalysisResultClausal& analysis_result);
		
private:
	void Initialise(Disjunction& learned_literals);
	void CleanUp(Disjunction& learned_literals);
	bool IsLiteralDominated(BooleanLiteral candidate);

	SolverState &state_;

	int64_t total_removed_literals_, num_minimisation_calls_, num_literals_before_minimising_, root_literals_removed_; //helper variables used to gather statistics
	DirectlyHashedBooleanVariableLabeller labels_;
	DirectlyHashedIntegerSet allowed_decision_levels_;
};
} //end namespace Pumpkin