#pragma once

#include "../Utilities/boolean_literal.h"
#include "../Utilities/boolean_variable.h"
#include "../Utilities/key_value_heap.h"
#include "../Utilities/directly_hashed_integer_set.h"
#include "../Utilities/bit_string_map.h"

#include <vector>
#include <stack>

namespace Pumpkin
{

class SolverState;
/*
A priority queue with the logic of the VSIDS variable selection in SAT solvers.
It is a lazy data structure: variables that have already been assigned values might be present during execution.
It is best to access it through the SATsolverState when calling for the best variable
*/
class VariableSelector
{
public:
	//create a variable selector that considers 'num_variables' Boolean variables with activities set to zero.
	VariableSelector(int num_variables, double decay_factor); 
	
	void BumpActivity(BooleanVariable boolean_variable); //bumps the activity of the Boolean variable
	void BumpActivity(BooleanVariable variable, double bump_multiplier); //bumps the variable but multiplies the increment by bump_multiplier. More efficient than repeatedly calling the above method.
	void DecayActivities();//decays the activities of all variables
	
	BooleanVariable PeekNextVariable(SolverState * state);//returns the next unassigned variable, or returns the undefined literal if there are no unassigned variables left. This method does +not+ remove the variable.
	void Remove(BooleanVariable); //removes the Boolean variable (temporarily) from further consideration. Its activity remains recorded internally and is available upon readding it to the data structure. The activity can still be subjected to DecayActivities(). O(logn)

	void Readd(BooleanVariable); //readd the Boolean variable into consideration. This call is ignored if the variable is already present. Assumes it was present in the data structure before. Its activity is kept to the previous value used before Remove(bv) was called. O(logn)
	void Grow(); //increases the number of variables considered by one. The new variable will have its activity assigned to zero.

	int Size() const;
	bool IsVariablePresent(BooleanVariable) const; //returns whether or not the variable is a candidate for selection

	void Reset(int seed = -1); //resets all activities of the variable currently present to zero. The seed is used to set the initial order of variables. Seed = -1 means variables are ordered by their index.

//private:
	void RescaleActivities();//divides all activities with a large number when the maximum activity becomes too large

	KeyValueHeap heap_; //the heap stores indicies of the variables minus one. The minus is applied since the heap operates in the range [0, ...) while variables are indexed [1, ...). Todo should consider changing this at some point.
	double increment_, max_threshold_, decay_factor_;

  BitStringMap bit_strings_;
};

} //end Pumpkin namespace