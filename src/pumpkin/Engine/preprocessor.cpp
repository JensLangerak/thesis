#include "preprocessor.h"

namespace Pumpkin
{
void Preprocessor::RemoveLiteralsWithFixedAssignmentFromObjective(ConstraintSatisfactionSolver& solver, LinearBooleanFunction& objective_function)
{
	static std::vector<BooleanLiteral> literals_to_remove;
	literals_to_remove.clear();
	int64_t lb_increase = 0;
	int64_t weighted_literals_implied = 0;
	//we first collect the literals that need to be removed
	//and then in next loop do we remove them
	//this must be done in two loops since while we are iterating through the literals, we should not change their value (undefined behaviour if we add/remove a new objective literal while iterating)
	for (BooleanLiteral objective_literal : objective_function)
	{
		if (solver.state_.assignments_.IsAssigned(objective_literal))
		{
			int64_t weight = objective_function.GetWeight(objective_literal);
			weighted_literals_implied += weight;
			runtime_assert(solver.state_.assignments_.IsRootAssignment(objective_literal));
			literals_to_remove.push_back(objective_literal); //make note of the literal so that it can be removed afterwards
			//increase the lower bound only if the literal was forcing a violation of the objective, otherwise we can simply ignore the literal
			if (solver.state_.assignments_.IsAssignedTrue(objective_literal))
			{
				lb_increase += weight;
				objective_function.IncreaseConstantTerm(weight);
			}
		}
	}
	//now actually remove the literals
	for (BooleanLiteral literal : literals_to_remove) { objective_function.RemoveLiteral(literal); }

	if (weighted_literals_implied > 0) { std::cout << "c preprocessor trivial lb increase " << lb_increase << ", trivials objective removals " << weighted_literals_implied << "\n"; }
}

bool Preprocessor::FixLiteralsWithLargeWeights(ConstraintSatisfactionSolver& solver, LinearBooleanFunction& objective_function, int64_t upper_bound)
{
	static std::vector<BooleanLiteral> literals_to_remove;
	literals_to_remove.clear();
	int64_t hardening = 0;
	//only consider literals with weights less or equal to the upper bound
	//literals with high weight are set to false in the solver
	for (BooleanLiteral objective_literal : objective_function)
	{
		int64_t weight = objective_function.GetWeight(objective_literal);
		if (weight + objective_function.GetConstantTerm() > upper_bound) //todo >= would work here, but then it makes the solver unsat - not a problem in principle but could be awkward with lexicographical optimisation, think about this. Make separate sat calls for these?
		{
			bool success = solver.state_.AddUnitClause(~objective_literal);
			if (!success) { return false; }
			literals_to_remove.push_back(objective_literal);
			hardening += weight;
		}
	}
	//now actually remove the literals
	for (BooleanLiteral literal : literals_to_remove) { objective_function.RemoveLiteral(literal); }

	if (hardening > 0) { std::cout << "c preprocessor harden: " << hardening << "\n"; }

	return true;
}
}
