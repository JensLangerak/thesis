#include "learned_clause_minimiser.h"
#include "../Utilities/runtime_assert.h"

#include <iostream>
#include <vector>

namespace Pumpkin
{
LearnedClauseMinimiser::LearnedClauseMinimiser(SolverState& state):
	state_(state),
	total_removed_literals_(0),
	num_minimisation_calls_(0),
	labels_(0),
	allowed_decision_levels_(0),
	num_literals_before_minimising_(0),
	root_literals_removed_(0)
{
}

void LearnedClauseMinimiser::RemoveImplicationGraphDominatedLiterals(ConflictAnalysisResultClausal& analysis_result)
{
	Disjunction& learned_literals = analysis_result.learned_clause_literals;

	Initialise(learned_literals);	
	size_t end_position(0);
	for (size_t i = 0; i < learned_literals.size(); i++)
	{
		BooleanLiteral learned_literal = learned_literals[i];
		//note that it does not make sense to remove the propagated literal
		if (learned_literal == analysis_result.propagated_literal || IsLiteralDominated(learned_literal) == false)
		{
			learned_literals[end_position] = learned_literal;
			end_position++;
		}
		root_literals_removed_ += ((state_.assignments_.GetAssignmentLevel(learned_literal) == 0) && (learned_literal != analysis_result.propagated_literal));
	}
	learned_literals.resize(end_position);
	CleanUp(learned_literals);

	assert(analysis_result.CheckCorrectnessAfterConflictAnalysis(state_));
}

void LearnedClauseMinimiser::Initialise(Disjunction& learned_literals)
{
	num_minimisation_calls_++;
	num_literals_before_minimising_ = learned_literals.size();
	//set the data structures to the appropriate size, e.g., in case new variables have been created since last time
	labels_.Resize(state_.GetNumberOfVariables()+1); //+1 is needed since we do not use variables with index 0
	allowed_decision_levels_.Resize(state_.GetNumberOfVariables() + 1);

	for (BooleanLiteral literal : learned_literals)
	{
		labels_.AssignLabel(literal.Variable(), 1);
		allowed_decision_levels_.Insert(state_.assignments_.GetAssignmentLevel(literal));
	}
}

void LearnedClauseMinimiser::CleanUp(Disjunction &learned_literals)
{
	total_removed_literals_ += (num_literals_before_minimising_ - learned_literals.size());
	labels_.Clear();
	allowed_decision_levels_.Clear();
}

bool LearnedClauseMinimiser::IsLiteralDominated(BooleanLiteral candidate)
{
	runtime_assert(state_.assignments_.IsAssigned(candidate));

	if (state_.assignments_.GetAssignmentLevel(candidate) == 0) { return true; } //note that root assignments are considered dominated
	if (state_.assignments_.IsDecision(candidate.Variable())) { return false; }
	if (labels_.IsAssignedSpecificLabel(candidate.Variable(), 0)) { return false; }
	if (!allowed_decision_levels_.IsPresent(state_.assignments_.GetAssignmentLevel(candidate))) { return false; }
	
	PropagatorGeneric *propagator = state_.assignments_.GetAssignmentPropagator(candidate.Variable());
	ExplanationGeneric* explanation;
	if (state_.assignments_.IsAssignedTrue(candidate)){ explanation = propagator->ExplainLiteralPropagation(candidate, state_); }
	else { explanation = propagator->ExplainLiteralPropagation(~candidate, state_); }
	
	for (int i = 0; i < explanation->Size(); i++)
	{
		BooleanLiteral reason_literal = (*explanation)[i];
		BooleanVariable reason_variable = reason_literal.Variable();

		if (labels_.IsAssignedSpecificLabel(reason_variable, 1)) { continue; }

		if (IsLiteralDominated(reason_literal) == false)
		{
			runtime_assert(!labels_.IsLabelled(reason_variable) || labels_.IsAssignedSpecificLabel(reason_variable, 0));
			labels_.AssignLabel(reason_variable, 0);
			return false;
		}
	}
	//if all the checks above passed, the candidate literal is implied by the other literals and can be removed
	labels_.AssignLabel(candidate.Variable(), 1);
	runtime_assert(allowed_decision_levels_.IsPresent(state_.assignments_.GetAssignmentLevel(candidate.Variable())));
	return true;
}

}//end namespace Pumpkin