#include "conflict_analysis_result_clausal.h"

namespace Pumpkin
{
ConflictAnalysisResultClausal::ConflictAnalysisResultClausal(std::vector<BooleanLiteral>& learned_literals, BooleanLiteral propagated_literal, int backtrack_level):
	learned_clause_literals(learned_literals),
	propagated_literal(propagated_literal),
	backtrack_level(backtrack_level)
{
	runtime_assert(!learned_clause_literals.empty());
	runtime_assert(!propagated_literal.IsUndefined());
	runtime_assert(backtrack_level >= 0);
}

bool ConflictAnalysisResultClausal::CheckCorrectnessAfterConflictAnalysis(SolverState& state)
{
	//+the propagating literal is the literal at the highest level and the highest level is the current level
	assert(state.GetCurrentDecisionLevel() == state.assignments_.GetAssignmentLevel(propagated_literal));
	for (BooleanLiteral current_literal : learned_clause_literals)
	{
		//+there is only one literal at the highest decision level
		assert(state.assignments_.GetAssignmentLevel(current_literal) < state.GetCurrentDecisionLevel() || current_literal == propagated_literal);
		//+all literals apart from the propagating literal are assigned false
		assert(state.assignments_.IsAssignedFalse(current_literal) || current_literal == propagated_literal);
	}
	return true;
}
}//end namespace Pumpkin
