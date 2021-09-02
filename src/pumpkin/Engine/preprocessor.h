#pragma once

#include "constraint_satisfaction_solver.h"
#include "../Utilities/boolean_literal.h"
#include "../Utilities/linear_boolean_function.h"


#include <vector>

namespace Pumpkin
{
class Preprocessor
{
public:
	static void RemoveLiteralsWithFixedAssignmentFromObjective(ConstraintSatisfactionSolver& solver, LinearBooleanFunction& objective_function);
	static bool FixLiteralsWithLargeWeights(ConstraintSatisfactionSolver& solver, LinearBooleanFunction& objective_function, int64_t upper_bound);
};
}