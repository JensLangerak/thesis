#pragma once

#include "solver_state.h"
#include "../Utilities/boolean_literal.h"
#include "../Utilities/runtime_assert.h"

#include <vector>

namespace Pumpkin
{

struct ConflictAnalysisResultClausal
{
	ConflictAnalysisResultClausal(std::vector<BooleanLiteral>& learned_literals, BooleanLiteral propagated_literal, int backtrack_level);
	
	bool CheckCorrectnessAfterConflictAnalysis(SolverState &state); //debugging method: performs sanity checks to see if the instance has been created correctly

	std::vector<BooleanLiteral> learned_clause_literals;
	BooleanLiteral propagated_literal;
	int backtrack_level;
};

}//end namespace Pumpkin