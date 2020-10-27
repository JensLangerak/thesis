#pragma once

#include "../Basic Data Structures/assignments.h"
#include "../Basic Data Structures/boolean_literal.h"
#include "../Basic Data Structures/boolean_variable.h"
#include "../Basic Data Structures/cumulative_moving_average.h"
#include "../Basic Data Structures/problem_specification.h"
#include "../Basic Data Structures/simple_moving_average.h"
#include "../Basic Data Structures/small_helper_structures.h"
#include "../Basic Data Structures/solver_parameters.h"
#include "../Basic Data Structures/trail_list.h"
#include "../Propagators/Cardinality/propagator_cardinality.h"
#include "../Propagators/Cardinality/propagator_cardinality2.h"
#include "../Propagators/Clausal/propagator_clausal.h"
#include "../Propagators/Pseudo-Boolean/propagator_pseudo_boolean_constraints.h"
#include "value_selector.h"
#include "variable_selector.h"

#include <list>
#include <vector>

namespace Pumpkin
{

class PropagatorGeneric; //need a forward declaration since propagators use the state class

//A class representing the internal state of the SAT solver. 
class SolverState
{
public:
	SolverState(int64_t num_Boolean_variables, SolverParameters&);

	//increases the current decision level and updates internal data structures accordingly
	//should only have one decision literal per decision level. Todo should add a check to ensure this condition.
	void IncreaseDecisionLevel();

//methods for enqueuing and variable assignment--------------------

	//places the literal in the propagation queue, noting its assignment was the result of a decision rather than a propagation
	//Assumes the corresponding variable has not yet been set
	void EnqueueDecisionLiteral(BooleanLiteral); 
	
	//places the literal in the queue, noting it was a result of the propagator given in input
	bool EnqueuePropagatedLiteral(BooleanLiteral propagated_literal, PropagatorGeneric *responsible_propagator, uint64_t code);
		
	//propagates all the enqueued learned_clause_literals and returns the first propagator that reported a conflict or NULL if no conflict was detected
	PropagatorGeneric * PropagateEnqueued(); 

//methods for backtracking--------------------
	//reverts all assignments done up to level 'backtrack_level'  (does not unassign variables from 'backtrack_level')
	//After calling the method, the current decision level will be set to 'backtrack_level'
	//assumes the state is not at 'backtrack_level'. Use reset to backtrack to root to avoid testing if at backtrack level zero
	void Backtrack(int backtrack_level);
	//backtracks at level zero. Note that level zero assignments are still kept.
	void Reset();
	//Reverts assignments done in the current level
	void BacktrackOneLevel();
	//Removes the last assignment. Note that the assignment could have been done as part of a decision or a propagation. 
	//To remove all assignments from the current level see BacktrackOneLevel()
	void UndoLastAssignment();

//methods for getting information about the state--------------------

	//returns the unassigned variable with the highest vsids score
	//it changes the state of the internal state of the variable heap since it removes and readds the variable
	//note that if there are multiple variables with the same score, calling this method in succession may result in different variables with the same score
	//in case there are no variable left, the method returns an undefined literal
	//todo should enabling retriving the top element of the heap without needing to remove
	BooleanVariable GetHighestActivityUnassignedVariable();

	//if there are no decision literals on the trail, returns an undefined literal
	BooleanLiteral getLastDecisionLiteralOnTrail() const;
	//returns the decision that was made at level 'decision_level'. Assumes the decision level is at most the current decision level. At root level, returns the undefined literal
	BooleanLiteral GetDecisionLiteralForLevel(int decision_level) const; 
	
	int GetHighestDecisionLevelForLiterals(std::vector<BooleanLiteral> &literals) const;

	//returns an integer representing the current decision level
	int GetCurrentDecisionLevel() const; 

	//returns the number of assigned variables. Note this includes both decision and propagated variables. 
	size_t GetNumberOfAssignedVariables() const;
	//return the total number of Boolean variables in the problem (note that variables do not necessarily need to be present in the constraints)
	size_t GetNumberOfVariables() const;

	BooleanLiteral GetLiteralFromTrailAtPosition(size_t index) const;
	//returns a literal placed on the trail. In this case the trail behaves like a stack: 
	//[0] is the most recently pushed literal, [1] is the second most recent, and so forth.
	BooleanLiteral GetLiteralFromTheBackOfTheTrail(size_t index) const;

	//returns the satisfying assignment present in the current state
	//should only be called if the assignment has been built
	std::vector<bool> GetOutputAssignment() const;

	//returns if all variables of the problem have been assigned a truth value. Internally might update lazy data structures
	bool IsAssignmentBuilt(); 
	
	void PrintTrail() const;

//methods for adding constraints--------------------

	//creates the next Boolean variable and grows internal data structures appropriately
	//should always be called when creating a new Boolean variable
	BooleanVariable CreateNewVariable();

	//creates additional variables to support variable indicies from the current index up until the given value (inclusive)
	//executes multiple calls of 'CreateNewVariable'
	void CreateVariablesUpToIndex(int largest_variable_index);

	//adding unit clauses needs special case. Before solving these methods can be used. To add during solving, use AddUnitClauseDuringSearch
	void AddUnitClause(BooleanLiteral&);
	void AddUnitClauses(std::vector<BooleanLiteral>& units);
	//adds a unit clause during the search. Currently this is done by restarting to the root and forcing the variable assignment on the trail and propagates other literals if necessary
	bool AddUnitClauseDuringSearch(BooleanLiteral);

	//only use these methods before solving.
	void AddClause(std::vector<BooleanLiteral>& literals);
	void AddBinaryClause(BooleanLiteral, BooleanLiteral);
	void AddTernaryClause(BooleanLiteral, BooleanLiteral, BooleanLiteral);
	void AddImplication(BooleanLiteral, BooleanLiteral); //add clause (a -> b)	

	//adds a learned clause_ to the database based on the input literals. 
	//the clause_ will be added temporarily, unless it has an LBD score of two or less, in which case it is added permanently. 
	//returns a pointer to the added clause_
	TwoWatchedClause * AddLearnedClauseToDatabase(std::vector<BooleanLiteral> &literals); 
	
	//The input lbd is the lbd of the learned clause_. Assumes this method is called prior to backtracking and adding the learned clause_ to the database. 
	//updates the moving average data structures that are used to determined if restarts are to take place.
	void UpdateMovingAveragesForRestarts(int learned_clause_lbd); 

//public class variables--------------------------for now these are kept as public, but at some point most will be moved into the private section
	VariableSelector variable_selector_;
	ValueSelector value_selector_;
	Assignments assignments_;

	//propagators
	PropagatorClausal propagator_clausal_;
	PropagatorPseudoBooleanConstraints propagator_pseudo_boolean_;
  PropagatorCardinality propagator_cardinality_;
  PropagatorCardinality2 propagator_cardinality2_;

  //data structures that control restarts
	SimpleMovingAverage simple_moving_average_block, simple_moving_average_lbd;
	CumulativeMovingAverage cumulative_moving_average_lbd;

  void AddCardinality(Pumpkin::CardinalityConstraint &constraint);
  void AddCardinality2(Pumpkin::CardinalityConstraint &constraint);

  TrailList<BooleanLiteral>::Iterator GetTrailEnd();

        TrailList<BooleanLiteral>::Iterator GetTrailBegin();

        bool InsertPropagatedLiteral(BooleanLiteral propagated_literal,
                                      PropagatorGeneric *responsible_propagator,
                                      uint64_t code, int decision_level);
        void FullReset();

        void ResetPropagatorsToLevel();

      private:
	//performs an assignment to make the literal true. 
	//Note that the corresponding variable is considered to be assigned a value (0 if the literal was negative, 1 if the literal was positive). Used internally, consider using 'enqueue' instead.
        void MakeAssignment(BooleanLiteral literal, PropagatorGeneric *responsible_propagator, uint64_t code);
  void MakeAssignment(BooleanLiteral literal, PropagatorGeneric *responsible_propagator, uint64_t code, int decision_level);

  //private class variables--------------------------
//	std::list<BooleanLiteral> trail_;
        TrailList<BooleanLiteral> trail_;
//	std::vector<std::list<BooleanLiteral>::iterator> trail_delimiter_; //[i] is the position where the i-th decision level ends (exclusive) on the trail.
        std::vector<TrailList<BooleanLiteral>::Iterator> trail_delimiter_;

	int decision_level_;
};

} //end Pumpkin namespace