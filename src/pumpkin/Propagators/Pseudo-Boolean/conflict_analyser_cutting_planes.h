#ifndef CONFLICT_ANALYSER_CUTTING_PLANES_H
#define CONFLICT_ANALYSER_CUTTING_PLANES_H

#include "watched_pseudo_boolean_constraint.h"
#include "../propagator_generic.h"
#include "../../Basic Data Structures/small_helper_structures.h"
#include "../../Basic Data Structures/learned_pseudo_boolean_constraint.h"

namespace Pumpkin
{

class SolverState;

//this is the results class, not the analyser -> need to clarify this, the name is difficult to read
struct ConflictAnalysisResultPseudoBoolean
{
	ConflictAnalysisResultPseudoBoolean(LearnedPseudoBooleanConstraint &learned_constraint, int b) :
		learned_pseudo_boolean(learned_constraint), backtrack_level(b)
	{}

	ConflictAnalysisResultPseudoBoolean(const ConflictAnalysisResultPseudoBoolean &source):
		learned_pseudo_boolean(source.learned_pseudo_boolean), backtrack_level(source.backtrack_level)
	{}

	LearnedPseudoBooleanConstraint learned_pseudo_boolean;
	std::vector<BooleanLiteral> forced_assignments_at_root_node; //todo: I think I want to remove this. If there are root assignments, then the backtrack level could be set to zero
	std::vector<BooleanLiteral> propagations_at_backtrack_level;
	int backtrack_level;
};

class ConflictAnalyserCuttingPlanes
{
public:
	ConflictAnalyserCuttingPlanes(size_t num_variables);

	ConflictAnalysisResultPseudoBoolean AnalyseConflict(PropagatorGeneric * conflict_propagator, SolverState &state);
	void ProcessConflictAnalysisResult(ConflictAnalysisResultPseudoBoolean & result, SolverState &state);

	void Grow();

private:
	void InitialiseConflictAnalysis(PropagatorGeneric *conflict_propagator, SolverState &state);
	void ProcessConflictPropagator(PropagatorGeneric *conflict_propagator, BooleanLiteral propagated_literal, SolverState &state); //during AnalyseConflict_old, effectively performs resolution, i.e. goes through the 'reason' literals to either 1) add them to the learned_clause_literals_ vector if their decision level is greater than the current level, or 2) increase the num_current_decision_level_literals_ if their decision is at the current level. Bumps variable activity. Updates 'backtrack_level'. Uses seen_ as well. Meant to be used within 'AnalyseConflict_old'.
	BooleanLiteral FindNextReasonLiteralOnTheTrail(SolverState &state);
	WatchedPseudoBooleanConstraint * AddLearnedPseudoBooleanConstraintToDatabase(LearnedPseudoBooleanConstraint&, SolverState&);
	uint64_t GreatestCommonDivisor(uint64_t a, uint64_t b);

	//in the future these three propagation methods will probably be changed - the propagators do the same thing but on a different data structure
	std::vector<BooleanLiteral> GetConflictingPropagation(SolverState& state);
	std::vector<BooleanLiteral> GetNewRootPropagations(SolverState& state);//the possible issue is that it can propagate at the root something conflicting, but I am not sure, I think this cannot happen since it cannot be the reason
	int64_t ComputeSlack(SolverState& state);

	void ClearDataStructures();
	bool CheckDataStructures();

	//cutting planes conflict analysis
	std::vector<bool> seen_; //I think this variable is not used anymore (temporarily?)
	std::vector<BooleanVariable> encountered_variables_;

	int num_current_decision_level_literals_;
	int num_trail_literals_examined_;
	LearnedPseudoBooleanConstraint learned_pseudo_boolean_;
};

} //end Pumpkin namespace

#endif // !CONFLICT_ANALYSER_CUTTING_PLANES_H