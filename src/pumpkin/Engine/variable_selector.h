#ifndef VARIABLE_SELETOR_H
#define VARIABLE_SELECTOR_H

#include "../Basic Data Structures/boolean_variable.h"
#include "../Basic Data Structures/key_value_heap.h"

namespace Pumpkin
{

/*
A helper class for implementing the logic of the VSIDS variable selection in SAT solvers.
It is a lazy data structure: variables that have already been assigned values might be present during execution.
It is best to access it through the SATsolverState when calling for the best variable
*/
class VariableSelector
{
public:
	VariableSelector(int num_variables); //create a variable selector that considers 'num_variables' Boolean variables with activities set to zero. Internally 'reserve' entry spaces will be allocated.
	
	void BumpActivity(BooleanVariable boolean_variable); //bumps the activity of the Boolean variable
	void DecayActivities();//decays the activities of all variables
	
	BooleanVariable PopHighestActivityVariable(); //returns the Boolean variable with the highest activity, and removes it from further consideration. If no variable is present, returns the undefined variable instead. O(logn)
	
	void Remove(BooleanVariable); //removes the Boolean variable (temporarily) from further consideration. Its activity remains recorded internally and is available upon readding it to the data structure. The activity can still be subjected to DecayActivities(). O(logn)
	void Readd(BooleanVariable); //readd the Boolean variable into consideration. This call is ignored if the variable is already present. Assumes it was present in the data structure before. Its activity is kept to the previous value used before Remove(bv) was called. O(logn)
	
	int Size() const;

	void Grow(); //increases the number of variables considered by one. The new variable will have its activity assigned to zero.

	bool IsVariablePresent(BooleanVariable) const; //returns whether or not the variable is a candidate for selection

//private:
	void RescaleActivities();//divides all activities with a large number when the maximum activity becomes too large
	
	KeyValueHeap heap_;
	double increment_, max_threshold_, decay_factor_;
};

} //end Pumpkin namespace

#endif // !VARIABLE_SELETOR_H