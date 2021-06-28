#pragma once

#include "solver_state.h"
#include "conflict_analysis_result_clausal.h"
#include "learned_clause_minimiser.h"
#include "../Utilities/boolean_variable.h"
#include "../Utilities/boolean_literal.h"
#include "../Utilities/problem_specification.h"
#include "../Utilities/solver_parameters.h"
#include "../Utilities/small_helper_structures.h"
#include "../Utilities/simple_moving_average.h"
#include "../Utilities/cumulative_moving_average.h"
#include "../Utilities/counters.h"
#include "../Utilities/stopwatch.h"
#include "../Utilities/solver_output.h"
#include "../Utilities/directly_hashed_integer_set.h"
#include "../Utilities/parameter_handler.h"
#include "../Utilities/luby_sequence_generator.h"

#include <vector>
#include <iostream>
#include <limits>

namespace Pumpkin
{

class ConstraintSatisfactionSolver
{
public:
	ConstraintSatisfactionSolver(ParameterHandler& parameters);
	ConstraintSatisfactionSolver(ProblemSpecification* problem_specification, ParameterHandler& parameters);
	SolverOutput Solve(double time_limit_in_seconds = std::numeric_limits<double>::max()); //solves the formula currently in the solver and returns a vector where the i-th entry denotes if the literal was true or false (empty vector for unsat formulas)
	SolverOutput Solve(std::vector<BooleanLiteral> &assumptions, double time_limit_in_seconds = std::numeric_limits<double>::max());

	std::string GetStatisticsAsString();

	SolverState state_; //todo move to private

private:

	enum class SolverExecutionFlag { SAT, UNSAT, UNSAT_UNDER_ASSUMPTIONS, TIMEOUT};
	SolverExecutionFlag SolveInternal(std::vector<BooleanLiteral>& assumptions, double time_limit_in_seconds = std::numeric_limits<double>::max());

	void Initialise(double time_limit_in_seconds, std::vector<BooleanLiteral>& assumptions);

	bool AreAllAssumptionsSet();
	BooleanLiteral PeekNextAssumption();
	BooleanLiteral PeekNextDecisionLiteral();
	
//conflict analysis methods--------------in the future these methods will be moved to a separate class to make it easier to integrate different variants of conflict analysis

	Disjunction ExtractCore(BooleanLiteral falsified_assumption);

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
	//i.e., adds the learned clause to the database, backtracks, enqueues the propagated literal, propagates, and updates internal data structures for simple moving averages
	PropagatorGeneric * ProcessConflictAnalysisResult(ConflictAnalysisResultClausal& result);
	
	//when the conflict analysis learns a clause that forces one of the assumptions to change, this method is invoked to compute the unsatisfiable core
	std::vector<BooleanLiteral> RefineConflictAnalysisResultIntoUnsatisfiableCore(ConflictAnalysisResultClausal &analysis_result);

	//std::vector<BooleanLiteral> RefineConflictAnalysisResultIntoUnsatisfiableCore(ConflictAnalysisResultClausal& analysis_result);

	//finds the next reason literal on the trail that is responsible for the current conflict. Used by AnalyseConflict
	BooleanLiteral FindNextReasonLiteralOnTheTrail();

	//todo - this does not do anything
	void PerformGlucoseVariableBumping();

	void ClearConflictAnalysisDataStructures();

	//restart methods--------------

		//true if it is determine that a restart should take place, false otherwise. Does not change the state.
	bool ShouldRestart();
	//Backtracks to the root and updates counters and removes learned clauses. 
	//Note this is different than calling Backtrack(0) since this method updates more information 
	//	and it may not restart to the root level if clause clean up will not happen and there are assumptions
	//	instead it restarts to the assumption level
	void PerformRestartDuringSearch();

	//similar as the previous method (PerformRestartDuringSearch), but always restarts to the root 
	void PerformRestartToRoot(); 
	
	//CleanUp is called at the end of the search
	//essentially performs a restart to the root
	void CleanUp();

	//small helper methods--------------
	void UpdateConflictCounters();
	//sets the unit clauses given in the formula. Does not respect the time limit
	bool SetUnitClauses();

	//produces a SolverOutput based on the state
	SolverOutput GenerateOutput();

	//debug methods------------------probably need to remove these in the future
	bool CheckConflictAnalysisDataStructures(); //a debug method
	bool IsSeenCleared() const;

//variables----------------
	struct InternalParameters
	{
		InternalParameters(ParameterHandler& parameters):
			bump_decision_variables(parameters.GetBooleanParameter("bump-decision-variables")),
			num_min_conflicts_per_restart(parameters.GetIntegerParameter("num-min-conflicts-per-restart"))
		{
			if (parameters.GetStringParameter("restart-strategy") == "glucose") { restart_strategy_ = RestartStrategy::GLUCOSE; }
			else if (parameters.GetStringParameter("restart-strategy") == "luby") { restart_strategy_ = RestartStrategy::LUBY; }
			else if (parameters.GetStringParameter("restart-strategy") == "constant") { restart_strategy_ = RestartStrategy::CONSTANT; }
			else { std::cout << "Error, unknown restart strategy: " << parameters.GetStringParameter("restart-strategy") << "\n"; exit(1); }
		
			restart_coefficient_ = parameters.GetIntegerParameter("restart-multiplication-coefficient");
		}

		bool bump_decision_variables;
		int num_min_conflicts_per_restart;
		enum class RestartStrategy { GLUCOSE, LUBY, CONSTANT } restart_strategy_;
		int restart_coefficient_;
	} internal_parameters_;

	Stopwatch stopwatch_;

	Counters counters_;

	LubySequenceGenerator luby_generator_;

	std::vector<BooleanLiteral> assumptions_;

	//helper variables used during conflict analysis
	LearnedClauseMinimiser learned_clause_minimiser_;
	bool use_glucose_bumping_;
	bool use_clause_minimisation_;
	Disjunction learned_clause_literals_;
	
	DirectlyHashedIntegerSet already_processed_;
	std::vector<BooleanVariable> glucose_bumper_candidates_;
	DirectlyHashedIntegerSet seen_;
	int backtrack_level_;
	int num_current_decision_level_literals_;
	int num_trail_literals_examined_;
        void LogUsedVars();
};

} //end Pumpkin namespace