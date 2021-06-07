#pragma once

#include "../Propagators/Clausal/propagator_clausal.h"
#include "../Propagators/Dynamic/watched_dynamic_constraint.h"
#include "../Propagators/Dynamic/watcher_dynamic_constraint.h"
#include "../Propagators/Pseudo-Boolean/propagator_pseudo_boolean_constraints.h"
#include "../Utilities/assignments.h"
#include "../Utilities/boolean_assignment_vector.h"
#include "../Utilities/boolean_literal.h"
#include "../Utilities/boolean_variable.h"
#include "../Utilities/cumulative_moving_average.h"
#include "../Utilities/parameter_handler.h"
#include "../Utilities/simple_moving_average.h"
#include "../Utilities/small_helper_structures.h"
#include "../Utilities/solver_parameters.h"
#include "value_selector.h"
#include "variable_selector.h"

#include <vector>

namespace Pumpkin
{

class PropagatorGeneric; //need a forward declaration since propagators use the state class

//A class representing the internal state of the SAT solver. 
class SolverState
{
public:
	SolverState(int64_t num_Boolean_variables, ParameterHandler&);
	~SolverState();

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

	//if there are no decision literals on the trail, returns an undefined literal
	BooleanLiteral getLastDecisionLiteralOnTrail() const;
	//returns the decision that was made at level 'decision_level'. Assumes the decision level is at most the current decision level. At root level, returns the undefined literal
	BooleanLiteral GetDecisionLiteralForLevel(int decision_level) const; 
	
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
	BooleanAssignmentVector GetOutputAssignment() const;

	//returns if all variables of the problem have been assigned a truth value. Internally might update lazy data structures
	bool IsAssignmentBuilt(); 
	bool IsPropagationComplete(); //returns true if all propagators have completed propagation for all literals on the trail, otherwise returns false
	
	void PrintTrail() const;

//methods for adding constraints--------------------

	//creates the next Boolean variable and grows internal data structures appropriately
	//should always be called when creating a new Boolean variable
	BooleanVariable CreateNewVariable();
	//creates additional variables to support variable indicies from the current index up until the given value (inclusive)
	//executes multiple calls of 'CreateNewVariable'
	void CreateVariablesUpToIndex(int largest_variable_index);

	//adds a unit class by backtracking to the root and forcing the literal assignment along with propagated assignments
	//returns false if adding caused a conflict at the root, i.e., the instance became unsatisfiable
	bool AddUnitClause(BooleanLiteral);		

	//restarts to the root level and forces the unit clause assignments one at a time and its propagations
	//Returns false if unsatisfiability was detected, otherwise return true
	bool AddUnitClauses(std::vector<BooleanLiteral>& units);
	//assumes that before calling this method, the solver state has completed propagation, i.e., there are no literals waiting in the propagation queue
	//adds a clause to the database and performs propagation 
	//returns the conflicting propagator if a conflict occured, otherwise returns NULL if no conflicts happened
	//if a conflicted took place, conflict analysis should be performed using the propagator to restore the solver to a non-conflicting state or report unsatisfiability
	PropagatorGeneric* AddClause(std::vector<BooleanLiteral>& literals);
	PropagatorGeneric* AddBinaryClause(BooleanLiteral, BooleanLiteral);
	PropagatorGeneric* AddTernaryClause(BooleanLiteral, BooleanLiteral, BooleanLiteral);
	PropagatorGeneric* AddImplication(BooleanLiteral, BooleanLiteral); //add clause (a -> b)	

	int AddPropagator(PropagatorGeneric* propagator); //adds the propagator to the state and returns its assigned ID (useful in case the propagator needs to be modified, see GetPropagator). Ownership of the propagator is transfered to SolverState, e.g., the destructor of SolverState will delete the propagator and it is assumed the propagator will not be deleted by some other method
	PropagatorGeneric* GetPropagator(int propagator_id);
	bool HasPropagator(PropagatorGeneric* propagator);

	//adds a learned clause_ to the database based on the input literals.
	//the clause_ will be added temporarily, unless it has an LBD score of two or less, in which case it is added permanently.
	//returns a pointer to the added clause_
	TwoWatchedClause * AddLearnedClauseToDatabase(std::vector<BooleanLiteral> &literals); 
	
	//The input lbd is the lbd of the learned clause_. Assumes this method is called prior to backtracking and adding the learned clause_ to the database. 
	//updates the moving average data structures that are used to determined if restarts are to take place.
	void UpdateMovingAveragesForRestarts(int learned_clause_lbd); 

	//these two are experimental methods, for now used only as part of the upper bounding algorithm with varying resolution
	//reverting the state will remove all learned clauses and permenant clauses added
	//other parts of the state are kept though
	//should implement a 'Shrink' method (opposite of Grow), but for now we ignore this issue (todo)
	void SetStateResetPoint();
	void PerformStateReset();

//public class variables--------------------------todo for now these are kept as public, but at some point most will be moved into the private section
	VariableSelector variable_selector_;
	ValueSelector value_selector_;
	Assignments assignments_;

	//------propagators
	PropagatorClausal propagator_clausal_;
	PropagatorPseudoBooleanConstraints propagator_pseudo_boolean_; //for now it is not used in the algorithm
	std::vector<PropagatorGeneric*> additional_propagators_;


	//data structures that control restarts
	SimpleMovingAverage simple_moving_average_block, simple_moving_average_lbd;
	CumulativeMovingAverage cumulative_moving_average_lbd;

        void AddScheduledEncodings();
      std::vector<WatchedDynamicConstraint*> scheduled_dynamic_constraints_;
private:
	//helper vector when adding binary/ternary clauses
	std::vector<BooleanLiteral> helper_vector_;

	//performs an assignment to make the literal true. 
	//Note that the corresponding variable is considered to be assigned a value (0 if the literal was negative, 1 if the literal was positive). Used internally, consider using 'enqueue' instead.
	void MakeAssignment(BooleanLiteral literal, PropagatorGeneric *responsible_propagator, uint64_t code); 
	
//state restoring variables, for now we only track the permanently added clauses but will do more in the future (maybe)
	int64_t saved_state_num_permanent_clauses_;
	int64_t saved_state_num_root_literal_assignments_;

//private class variables--------------------------
	std::vector<BooleanLiteral> trail_;
	std::vector<int> trail_delimiter_; //[i] is the position where the i-th decision level ends (exclusive) on the trail.
	bool use_binary_clause_propagator_;
	int decision_level_;

};

} //end Pumpkin namespace