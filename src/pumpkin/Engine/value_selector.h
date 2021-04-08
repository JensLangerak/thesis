#ifndef VALUE_SELECTOR_H
#define VALUE_SELECTOR_H

#include "../Basic Data Structures/boolean_literal.h"

#include <vector>

namespace Pumpkin
{

class ValueSelector
{
public:
	ValueSelector(int num_variables);
	bool SelectValue(BooleanVariable); //returns the suggested truth value assigned for the given variable
  void UpdatePolarity(BooleanVariable, bool, bool);//sets the polarity of the input variable to the given truth value. As a result, SelectValue will now suggest the input truth value for the variable
  void UpdatePolarity(BooleanVariable l, bool b) {UpdatePolarity(l, b, false);};//sets the polarity of the input variable to the given truth value. As a result, SelectValue will now suggest the input truth value for the variable
	void Grow(); //increases the number of variables considered by one.

	void SetPolaritiesToFalse();
	void UnfreezeAll();

	void SetAndFreezeValue(BooleanLiteral);
	void PredetermineValues(std::vector<bool> &solution);
	void InitialiseValues(std::vector<bool> &solution);	

private:
	std::vector<bool> values_;
	bool are_values_predetermined_;
	std::vector<bool> is_frozen_;
};

} //end Pumpkin namespace

#endif // !VALUE_SELECTOR_H