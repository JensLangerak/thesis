#pragma once

#include "../Basic Data Structures/boolean_literal.h"
#include "../Basic Data Structures/boolean_variable.h"
#include "../Basic Data Structures/counters.h"
#include "../Basic Data Structures/cumulative_moving_average.h"
#include "../Basic Data Structures/problem_specification.h"
#include "../Basic Data Structures/simple_moving_average.h"
#include "../Basic Data Structures/small_helper_structures.h"
#include "../Basic Data Structures/solver_output.h"
#include "../Basic Data Structures/solver_parameters.h"
#include "../Basic Data Structures/stopwatch.h"
#include "../Engine/solver_state.h"
#include "../Propagators/Dynamic/Encoders/generalized_totaliser.h"

#include <iostream>
#include <limits>
#include <vector>

namespace Pumpkin
{

class ConstraintSatisfactionSolver
{
public:
	ConstraintSatisfactionSolver(ProblemSpecification& problem_specification, SolverParameters& parameters);
	SolverOutput Solve(double time_limit_in_seconds = std::numeric_limits<double>::max()); //solves the formula currently in the solver and returns a vector where the i-th entry denotes if the literal was true or false (empty vector for unsat formulas)

	void PrintStats();

	SolverState state_;

private:

	void Initialise(double time_limit_in_seconds);

	BooleanLiteral MakeDecision(); //return the next literal that should be set to true

//conflict analysis methods--------------in the future these methods will be moved to a separate class to make it easier to integrate different variants of conflict analysis

	//given a propagator that reported a conflict, attempts to restore the solver in a nonconflicting state. 
	//Internally it performs the necessary steps to resolve the conflict, 
	//i.e. learns and adds a new clause, enqueues and propagates the conflicting assignment literal, and update variable/clause activities
	//returns true if the conflict could be resolved; otherwise false
	bool ResolveConflict(PropagatorGeneric* conflict_propagator);

	//I think analyse conflict should be in the state and/or a separate conflict-analysing class to make it easier to allow different types of conflict analysis to be interchanged
	//analyses the conflict and returns the information to resolve the conflict. 
	//Does not change the state, apart from bumps activities	
	ConflictAnalysisResultClausal AnalyseConflict(PropagatorGeneric* conflict_propagator);

	//used during AnalyseConflict, effectively performs resolution, 
	//i.e. goes through the 'reason' literals to either 
	//1) add them to the learned_clause_literals_ vector if their decision level is greater than the current level, or 
	//2) increase the num_current_decision_level_literals_ if their decision is at the current level. 
	//Bumps variable activity. Updates 'backtrack_level'. Uses seen_ as well. 
	//if propagated literal is undefined, this initialises the conflict clause
	void ProcessConflictPropagator(PropagatorGeneric* conflict_propagator, BooleanLiteral propagated_literal);

	//changes the state based on the conflict analysis result given as input
	//i.e., adds the learned clause to the database, backtracks, enqueues the propagated literal, and updates internal data structures for simple moving averages
	void ProcessConflictAnalysisResult(ConflictAnalysisResultClausal& result);
        int log_conflict_id = 0;
        int log_conflict_ids[10] = {0,0,0,0,0,0,0,0,0,0};

	//finds the next reason literal on the trail that is responsible for the current conflict. Used by AnalyseConflict
	BooleanLiteral FindNextReasonLiteralOnTheTrail();

	//todo - this does not do anything
	void PerformGlucoseVariableBumping();

	void ClearConflictAnalysisDataStructures();

	//restart methods--------------

		//true if it is determine that a restart should take place, false otherwise. Does not change the state.
	bool ShouldRestart();
	//Backtracks to the root and updates counters and removes learned clauses. 
	//Note this is different than calling Backtrack(0) since this method updates more information.
	void PerformRestart();

	//small helper methods--------------
	void UpdateConflictCounters();
	//sets the unit clauses given in the formula. Does not respect the time limit
	bool SetUnitClauses();
	//produces a SolverOutput based on the state
	SolverOutput GenerateOutput();

	//debug methods------------------probably need to remove these in the future
	bool CheckConflictAnalysisDataStructures(); //a debug method
	bool IsSeenCleared() const;
	bool CheckDecisionLevelsLearnedLiteral(Disjunction& literals);

	//variables----------------
	Stopwatch stopwatch_;
	SolverParameters parameters_;
	Counters counters_;

	//helper variables used during conflict analysis
	bool use_glucose_bumping_;
	Disjunction learned_clause_literals_;
	std::vector<BooleanVariable> glucose_bumper_candidates_;
	std::vector<bool> seen_;
	int backtrack_level_;
	int num_current_decision_level_literals_;
	int num_trail_literals_examined_;


        //log info
        int total_encoded_conflicts = 0;
        int total_lits_in_conflicts = 0;
        int total_lbd_conflicts = 0;
        int lbd_conflict_count = 0;
        int lbd_histogram[21];
  int total_propagated_lits = 0;
  int total_encoded_lits = 0;
  int total_leaf_lits = 0;
        int total_max_path = 0;
        int total_max_same_constraints = 0;
        int total_non_constraint = 0;
        int total_encoded_conflicts_more_same_constraint=0;
        void LogNodeHits(int i);
        void UpdateHits(std::vector<int> &level_hits,
                        std::vector<int> &zero_hits, std::vector<int> &recent_hits, std::vector<int> &slightly_less_recent,int start_log_id, int index,
                        Pumpkin::GeneralizedTotaliser::Node *node);
};

} //end Pumpkin namespace