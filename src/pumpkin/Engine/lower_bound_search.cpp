#include "lower_bound_search.h"
#include "preprocessor.h"
#include "../Utilities/stopwatch.h"
#include "../Utilities/runtime_assert.h"
#include "../Pseudo-Boolean Encoders/cardinality_constraint_encoder_unary.h"

#include <set>

namespace Pumpkin
{
LowerBoundSearch::LowerBoundSearch(ParameterHandler& parameters)
{
	std::string parameter_stratification = parameters.GetStringParameter("stratification");
	
	if (parameter_stratification == "off") { stratification_strategy_ = StratificationStrategy::OFF; }
	else if (parameter_stratification == "basic") { stratification_strategy_ = StratificationStrategy::BASIC; }
	else if (parameter_stratification == "ratio") { stratification_strategy_ = StratificationStrategy::RATIO; }
	else { std::cout << "Stratification strategy \"" << parameter_stratification << "\" unknown!\n"; exit(1); }

	std::string parameter_encoding = parameters.GetStringParameter("cardinality-encoding");
	if (parameter_encoding == "totaliser") { cardinality_constraint_encoding_ = CardinalityConstraintEncoding::TOTALISER; }
	else if (parameter_encoding == "cardinality-network") { cardinality_constraint_encoding_ = CardinalityConstraintEncoding::CARDINALITY_NETWORK; }
	else { std::cout << "Encoding \"" << parameter_encoding << "\" unknown!\n"; exit(1); }

	use_weight_aware_core_extraction_ = parameters.GetBooleanParameter("weight-aware-core-extraction");
}

bool LowerBoundSearch::Solve(ConstraintSatisfactionSolver& solver, LinearBooleanFunction& objective_function, SolutionTracker& solution_tracker, int64_t lower_bound_previous_iterations_original_objective, double time_limit_in_seconds)
{
	if (time_limit_in_seconds <= 0.001) { return false; }
	if (solution_tracker.UpperBound() == objective_function.GetConstantTerm()) { return true; }

	std::cout << "c core-guided time: " << time_limit_in_seconds << "\n";

	Stopwatch stopwatch(time_limit_in_seconds);
	objective_function.ConvertIntoCanonicalForm();

	Preprocessor::RemoveLiteralsWithFixedAssignmentFromObjective(solver, objective_function);
	bool success = Preprocessor::FixLiteralsWithLargeWeights(solver, objective_function, solution_tracker.UpperBound());
	runtime_assert(success); //otherwise too easy, but could be the case

	int64_t weight_threshold = GetInitialWeightThreshold(objective_function);
	std::cout << "c \tinitial stratification threshold: " << weight_threshold << "\n";

	while (weight_threshold > 0)
	{
		CoreGuidedSearchWithWeightThreshold(weight_threshold, stopwatch, solver, objective_function, solution_tracker, lower_bound_previous_iterations_original_objective);
		weight_threshold = GetNextWeightThreshold(weight_threshold, objective_function);
		std::cout << "c \tnew stratification threshold: " << weight_threshold << "\n";
	}	

	std::cout << "c card network time: " << cardinality_network_encoder_.TotalTimeSpent() << "\n";
	std::cout << "c clauses added: " << cardinality_network_encoder_.TotalNumClausesAdded() << "\n";

	return stopwatch.IsWithinTimeLimit();
}

int64_t LowerBoundSearch::GetInitialWeightThreshold(const LinearBooleanFunction& objective_function)
{
	switch (stratification_strategy_)
	{
		case StratificationStrategy::OFF:
		{
			return 1;
		}
		case StratificationStrategy::BASIC:
		{
			return std::max(int64_t(1), objective_function.GetMaximumWeight());
		}
		case StratificationStrategy::RATIO:
		{
			int64_t large_weight = objective_function.GetMaximumWeight() + 1;
			return std::max(int64_t(1), GetNextWeightRatioStrategy(large_weight, objective_function));
		}
		default:
		{
			runtime_assert(1 == 2); return -1;
		}
	}
}

int64_t LowerBoundSearch::GetNextWeightThreshold(int64_t current_weight_threshold, const LinearBooleanFunction& objective_function)
{
	switch (stratification_strategy_)
	{
		case StratificationStrategy::OFF:
			return 0;

		case StratificationStrategy::BASIC:
			return objective_function.GetMaximumWeightSmallerThanThreshold(current_weight_threshold);

		case StratificationStrategy::RATIO:
			return GetNextWeightRatioStrategy(current_weight_threshold, objective_function);

		default:
			runtime_assert(1 == 2); return -1;
	}
}

int64_t LowerBoundSearch::GetNextWeightRatioStrategy(int64_t previous_weight_threshold, const LinearBooleanFunction& objective_function)
{
	//trivial case, if there are no literals in the objective, return zero to signal nothing needs to be done
	if (objective_function.NumObjectiveLiterals() == 0) { return 0; }
	//if we already tried every weight before, then return zero as the signal to stop
	if (previous_weight_threshold == 1) { return 0; }
	
	int64_t candidate_weight = previous_weight_threshold;
	std::set<int64_t> considered_weights;
	std::vector<BooleanLiteral> considered_literals;
	do
	{
		candidate_weight = objective_function.GetMaximumWeightSmallerThanThreshold(candidate_weight);
		considered_literals = objective_function.GetLiteralsWithWeightGreaterOrEqualToThreshold(candidate_weight);
		for (BooleanLiteral literal : considered_literals) { considered_weights.insert(objective_function.GetWeight(literal)); }
		//if we hit the ratio, we found the new weight, otherwise reiterate
		if (double(considered_literals.size()) / considered_weights.size() >= 1.25) 
		{ 
			return candidate_weight; 
		}
	} while (considered_literals.size() != objective_function.NumObjectiveLiterals());
	//if this return statement is reached, it means all literals are being considered
	return candidate_weight;
}

bool LowerBoundSearch::HardenAndFilterHardenedAssumptions(std::vector<BooleanLiteral>& assumptions, ConstraintSatisfactionSolver& solver, LinearBooleanFunction& objective_function, SolutionTracker& solution_tracker)
{
	int64_t sum_hardened_weights = 0;
	size_t i = 0; //todo can be implemented more efficiently with two pointers
	while (i < assumptions.size())
	{
		BooleanLiteral objective_literal = assumptions[i];
		int64_t weight = objective_function.GetWeight(objective_literal);
		if (objective_function.GetConstantTerm() + weight > solution_tracker.UpperBound())
		{
			bool success = solver.state_.AddUnitClause(~objective_literal);
			if (!success) { return false; }
			assumptions.erase(assumptions.begin() + i); //for now keeping the order of assumptions, removal could be done better
			objective_function.RemoveLiteral(objective_literal); //we can safely remove the literal from the objective
			sum_hardened_weights += weight;
		}
		else
		{
			i++;
		}
	}
	if (sum_hardened_weights > 0) { std::cout << "c hardening " << sum_hardened_weights << "\n"; }
	return true;
}

std::vector<BooleanLiteral> LowerBoundSearch::InitialiseAssumptions(int64_t weight_threshold, const LinearBooleanFunction& objective_function)
{
	static std::vector<BooleanLiteral> assumptions;
	assumptions = objective_function.GetLiteralsWithWeightGreaterOrEqualToThreshold(weight_threshold);
	for (size_t i = 0; i < assumptions.size(); i++) { assumptions[i] = ~assumptions[i]; }
	return assumptions;
}

int64_t LowerBoundSearch::GetMinimumCoreWeight(Disjunction& core_clause, const LinearBooleanFunction& objective_function)
{
	int64_t min_core_weight = objective_function.GetWeight(core_clause[0]);
	for (BooleanLiteral core_literal : core_clause)
	{
		min_core_weight = std::min(objective_function.GetWeight(core_literal), min_core_weight);
	}
	runtime_assert(min_core_weight >= 1);
	return min_core_weight;
}

void LowerBoundSearch::DecreaseCoreLiteralWeights(Disjunction& core_clause, LinearBooleanFunction& objective_function)
{
	int64_t min_core_weight = GetMinimumCoreWeight(core_clause, objective_function);
	for (BooleanLiteral core_literal : core_clause) { objective_function.DecreaseWeight(core_literal, min_core_weight); }
}

void LowerBoundSearch::ProcessCores(std::vector<Disjunction>& core_clauses, std::vector<int64_t> core_weights, ConstraintSatisfactionSolver& solver, LinearBooleanFunction& objective_function)
{
	for (size_t i = 0; i < core_clauses.size(); i++)
	{
		//std::vector<BooleanLiteral> soft_literals = 
		std::vector<BooleanLiteral> soft_literals;
		
		if (cardinality_constraint_encoding_ == CardinalityConstraintEncoding::TOTALISER)
		{
			soft_literals = totaliser_encoder_.SoftLessOrEqual(core_clauses[i], 1, solver.state_);
		}
		else if (cardinality_constraint_encoding_ == CardinalityConstraintEncoding::CARDINALITY_NETWORK)
		{
			soft_literals = cardinality_network_encoder_.SoftLessOrEqual(core_clauses[i], 1, solver.state_);
		}
		else
		{
			runtime_assert(1 == 2);
		}
		
		for (BooleanLiteral new_objective_literal : soft_literals)
		{
			runtime_assert(objective_function.GetWeight(new_objective_literal) == 0);
			objective_function.IncreaseWeight(new_objective_literal, core_weights[i]);
		}
	}
}

void LowerBoundSearch::FilterAssumptionsBasedOnWeightThreshold(std::vector<BooleanLiteral>& assumptions, const Disjunction& core_clause, int64_t weight_threshold, const LinearBooleanFunction& objective_function)
{
	for (BooleanLiteral core_literal : core_clause)
	{
		auto iter = std::find(assumptions.begin(), assumptions.end(), ~core_literal);
		runtime_assert(iter != assumptions.end());
		if (objective_function.GetWeight(core_literal) < weight_threshold) { assumptions.erase(iter); }
	}
}

void LowerBoundSearch::CoreGuidedSearchWithWeightThreshold(int64_t weight_threshold, Stopwatch& stopwatch, ConstraintSatisfactionSolver& solver, LinearBooleanFunction& objective_function, SolutionTracker& solution_tracker, int64_t lower_bound_previous_iterations_original_objective)
{
	runtime_assert(weight_threshold > 0);
	runtime_assert(solution_tracker.HasFeasibleSolution());

	if (stopwatch.TimeLeftInSeconds() <= 0.001) { return; }
	if (solution_tracker.UpperBound() == objective_function.GetConstantTerm()) { return; }

	std::vector<BooleanLiteral> assumptions;
	std::vector<Disjunction> cores;
	std::vector<int64_t> core_weights;

	while (stopwatch.IsWithinTimeLimit())
	{
		cores.clear();
		core_weights.clear();
		Preprocessor::RemoveLiteralsWithFixedAssignmentFromObjective(solver, objective_function);
		bool success =  Preprocessor::FixLiteralsWithLargeWeights(solver, objective_function, solution_tracker.UpperBound());
		runtime_assert(success);

		assumptions = InitialiseAssumptions(weight_threshold, objective_function);
		std::cout << "c ---\n";
		do
		{
			SolverOutput output = solver.Solve(assumptions, stopwatch.TimeLeftInSeconds());
			if (!stopwatch.IsWithinTimeLimit()) { std::cout << "core-guided timeout!\n"; break; } //important to break and not to return since the instances still needs to be reformulated according to the cores

			if (output.HasSolution())
			{
				std::cout << "c core-guided found a solution\n";
				runtime_assert(totaliser_encoder_.DebugCheckSatisfactionOfEncodedConstraints(output.solution));
				solution_tracker.UpdateBestSolution(output.solution);
				assumptions.clear();
			}
			else if (output.core_clause.empty() && output.ProvenInfeasible()) //this can happen thanks to hardening
			{
				runtime_assert(1 == 2); //cannot happen anymore, todo fix this part, hardening does not take into account the >= sign
				runtime_assert(cores.empty());
				//getting here means that the best solution found is the optimal solution
				int64_t lower_bound_update = solution_tracker.UpperBound() - objective_function.GetConstantTerm();
				objective_function.IncreaseConstantTerm(lower_bound_update);

				std::cout << "c unsat during core guided; optimal found\n";
				std::cout << "c LB = " << objective_function.GetConstantTerm() << "\n";
				return; //stop further search
			}
			else
			{
				std::cout << "c core size: " << output.core_clause.size() << "\n";
				std::cout << "c t = " << stopwatch.TimeElapsedInSeconds() << "\n";

				//some core post-processing
				//	increase lower bounds of the core
				//	try to remove literals -> 
				//		for example, try to get a core using only the literal that would lead to a worse solution if they are set 
				//		-> in principle this can be handled by hardening but then the solver may get into an UNSAT state which may be a problem with lexicographical optimisation
				runtime_assert(output.core_clause.size() > 0);
				int64_t core_weight = GetMinimumCoreWeight(output.core_clause, objective_function);
				DecreaseCoreLiteralWeights(output.core_clause, objective_function);
				objective_function.IncreaseConstantTerm(core_weight);
				cores.push_back(output.core_clause);
				core_weights.push_back(core_weight);
				bool success = HardenAndFilterHardenedAssumptions(assumptions, solver, objective_function, solution_tracker);
				runtime_assert(success);
				FilterAssumptionsBasedOnWeightThreshold(assumptions, output.core_clause, weight_threshold, objective_function);
				std::cout << "c LB = " << objective_function.GetConstantTerm() + lower_bound_previous_iterations_original_objective << "; num_assump = " << assumptions.size() << "\n";
			}
		} while (use_weight_aware_core_extraction_ && !assumptions.empty());

		if (cores.empty()) { break; }

		ProcessCores(cores, core_weights, solver, objective_function);
	}
}

}

