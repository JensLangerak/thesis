#include "upper_bound_search.h"
#include "../../logger/logger.h"
#include "../Utilities/stopwatch.h"
#include "preprocessor.h"

namespace Pumpkin
{
UpperBoundSearch::UpperBoundSearch(ParameterHandler& parameters) //:
//	upper_bound_propagator_(0)
{
	bool use_propagator = parameters.GetBooleanParameter("ub-propagator");
//	if (use_propagator) { upper_bound_propagator_ = new PropagatorCounterSinglePseudoBoolean(parameters.GetBooleanParameter("ub-propagator-bump")); }
        upper_bound_adder_ = parameters.optimisation_constraint_wrapper_;

	std::string varying_resolution_parameter = parameters.GetStringParameter("varying-resolution");
	if (varying_resolution_parameter == "off") { varying_resolution_strategy_ = VaryingResolutionStrategy::OFF; }
	else if (varying_resolution_parameter == "basic") { varying_resolution_strategy_ = VaryingResolutionStrategy::BASIC; }
	else if (varying_resolution_parameter == "ratio") { varying_resolution_strategy_ = VaryingResolutionStrategy::RATIO; }
	else { std::cout << "Varying resolution parameter \"" << varying_resolution_parameter << "\" unknown!\n"; exit(1); }

	std::string value_selection_parameter = parameters.GetStringParameter("value-selection");
	if (value_selection_parameter == "phase-saving") { value_selection_strategy_ = ValueSelectionStrategy::PHASE_SAVING; }
	else if (value_selection_parameter == "solution-guided-search") { value_selection_strategy_ = ValueSelectionStrategy::SOLUTION_GUIDED_SEARCH; }
	else if (value_selection_parameter == "optimistic") { value_selection_strategy_ = ValueSelectionStrategy::OPTIMISTIC; }
	else if (value_selection_parameter == "optimistic-aux") { value_selection_strategy_ = ValueSelectionStrategy::OPTIMISTIC_AUX; }
	else { std::cout << "Phase saving parameter \"" << value_selection_parameter << "\" unknown!\n"; exit(1); }
}

bool UpperBoundSearch::Solve(ConstraintSatisfactionSolver& solver, LinearBooleanFunction& objective_function, SolutionTracker& solution_tracker, int64_t lower_bound_previous_iterations_original_objective, double time_limit_in_seconds)
{
	if (time_limit_in_seconds <= 0.001) { return false; }
	if (solution_tracker.UpperBound() == objective_function.GetConstantTerm()) { return true; }

//        if (upper_bound_adder_ != nullptr && upper_bound_adder_->GetPropagator() != NULL && !solver.state_.HasPropagator(upper_bound_adder_->GetPropagator())) { solver.state_.AddPropagator(upper_bound_adder_->GetPropagator()); }
//	if (upper_bound_propagator_ != NULL && !solver.state_.HasPropagator(upper_bound_propagator_)) { solver.state_.AddPropagator(upper_bound_propagator_); }

	Stopwatch stopwatch(time_limit_in_seconds);

	Preprocessor::RemoveLiteralsWithFixedAssignmentFromObjective(solver, objective_function);
	bool success = Preprocessor::FixLiteralsWithLargeWeights(solver, objective_function, solution_tracker.UpperBound());
	runtime_assert(success); //otherwise the instance is too easy, but could be the case
	//if all literals are preprocessed no need to go further
	if (objective_function.NumObjectiveLiterals() == 0) { return true; }

	solver.state_.SetStateResetPoint();
	int64_t division_coefficient = GetInitialDivisionCoefficient(objective_function); //todo - I think when choosing the coefficient, we should take into account the solution we currently have. If say the coefficient is selected in a way that no literals are violating with weight greater than the coefficient, than there is no point is selecting that weight.
	while (division_coefficient > 0)
	{
		std::cout << "c division coefficient: " << division_coefficient << "\n";

		solver.state_.PerformStateReset();
		pseudo_boolean_encoder_.Clear();

		LinearBooleanFunction simplified_objective = GetVaryingResolutionObjective(division_coefficient, objective_function);
		LinearSearch(solver, simplified_objective, solution_tracker, objective_function.GetConstantTerm() + lower_bound_previous_iterations_original_objective, stopwatch.TimeLeftInSeconds());
		division_coefficient = GetNextDivisionCoefficient(division_coefficient, objective_function);
	}
	return stopwatch.IsWithinTimeLimit();
}

void UpperBoundSearch::LinearSearch(ConstraintSatisfactionSolver& solver, LinearBooleanFunction& objective_function, SolutionTracker& solution_tracker, int64_t lower_bound_original_objective, double time_limit_in_seconds)
{
	runtime_assert(solution_tracker.HasFeasibleSolution() && solution_tracker.UpperBound() >= lower_bound_original_objective);
	runtime_assert(objective_function.GetConstantTerm() == 0); //due to varying resolution we always have a zero constant term

	std::cout << "c linear search time: " << time_limit_in_seconds << "\n";	

	if (time_limit_in_seconds <= 0.001) { return; }
	if (solution_tracker.UpperBound() == lower_bound_original_objective) { return; }

	Stopwatch stopwatch(time_limit_in_seconds);
	solution_tracker.ExtendOptimalSolution(ComputeExtendedSolution(solution_tracker.GetBestSolution(), solver, stopwatch.TimeLeftInSeconds())); //we extend the solution to take into account auxiliary variables introduced by the core-guided phase
	int64_t internal_upper_bound = objective_function.ComputeSolutionCost(solution_tracker.GetBestSolution()); //this is the bound that will be used to update the upper bound. Note that when using varying resolution, the objective function provided might not be the same one used in the solution_tracker, so we need to track the internal upper bound separately
	
	Preprocessor::RemoveLiteralsWithFixedAssignmentFromObjective(solver, objective_function);
	bool success = Preprocessor::FixLiteralsWithLargeWeights(solver, objective_function, internal_upper_bound);
	runtime_assert(success); //I think this never happens but could happen

	if (internal_upper_bound == 0) { return; }

	std::vector<PairWeightLiteral> sum_literals = EncodeInitialUpperBound(
		solver,
		objective_function,
		internal_upper_bound - 1,
		time_limit_in_seconds);
	
	SetValueSelectorValues(solver, objective_function, solution_tracker.GetBestSolution());

	//todo check the stopping condition
	while (stopwatch.IsWithinTimeLimit() && solution_tracker.UpperBound() > lower_bound_original_objective)
	{
		SolverOutput output = solver.Solve(stopwatch.TimeLeftInSeconds());
		if (output.HasSolution())
		{
			runtime_assert(pseudo_boolean_encoder_.DebugCheckSatisfactionOfEncodedConstraints(output.solution));
			int64_t new_internal_upper_bound = objective_function.ComputeSolutionCost(output.solution);
//			runtime_assert(new_internal_upper_bound < internal_upper_bound);
			internal_upper_bound = new_internal_upper_bound;
						
			solution_tracker.UpdateBestSolution(output.solution); //note that this may fail if we introduced auxiliary variables during the core-guided phase: the aux variables might be set to higher values than they should, and setting them to zero does not change the original solution
			
			SetValueSelectorValues(solver, objective_function, output.solution);

			int new_upper_bound_on_free_terms = new_internal_upper_bound - objective_function.GetConstantTerm() - 1; //recall that core-guided phase might raise the lower bound, need to take that into account
			if (new_upper_bound_on_free_terms < 0) {
                          break; }
			bool success = StrengthenUpperBound(
				sum_literals,
				new_upper_bound_on_free_terms,
				solver);
			if (!success) {
                          break; }
		}
		else if (output.ProvenInfeasible())
		{
                  simple_sat_solver::logger::Logger::Log2("Proven infeasible: " + std::to_string(solution_tracker.UpperBound()));
			break;
		}
	}

//	if (upper_bound_propagator_ != NULL) { upper_bound_propagator_->Deactivate(); } // TODO
}

std::vector<PairWeightLiteral> UpperBoundSearch::EncodeInitialUpperBound(ConstraintSatisfactionSolver& solver, LinearBooleanFunction& objective_function, int64_t upper_bound, double time_limit_in_seconds)
{
	std::cout << "c linear search initial bound " << upper_bound << "\n";

//	if (upper_bound_propagator_ != NULL)
//	{
//		std::cout << "c using propagator for the upper bound constraint\n";
//		bool success = upper_bound_propagator_->Activate(solver.state_, objective_function, upper_bound);
//		runtime_assert(success); //not necessarily needs to hold but if even posting the constraint is unsat, then it is probably due to a bug
//		return std::vector<PairWeightLiteral>();
//	}
        if (upper_bound_adder_ != nullptr) {
          std::cout << "c using propagator for the upper bound constraint\n";
//          bool success = upper_bound_propagator_->Activate(solver.state_, objective_function, upper_bound);
          std::vector<BooleanLiteral> lits;
          std::vector<uint32_t > weights;
          for (auto l : objective_function) {
            lits.push_back(l);
            weights.push_back(objective_function.GetWeight(l));
          }
          //TODO encoder
          PseudoBooleanConstraint constraint(lits, weights, upper_bound, upper_bound_adder_);
          upper_bound_constraint_ = upper_bound_adder_->AddConstraintWithModifier(constraint, solver.state_);
	  return std::vector<PairWeightLiteral>();
        }

	//todo delegate this to the encoder?
	
	//for now the encoding time does not care about the stopwatch...need to terminate if we go overtime
	Stopwatch stopwatch(time_limit_in_seconds);
	std::vector<PairWeightLiteral> weighted_literals;
	for (BooleanLiteral literal : objective_function) 
	{ 
		weighted_literals.push_back(PairWeightLiteral(literal, objective_function.GetWeight(literal))); 
	}
	std::cout << "c num weighted lits: " << weighted_literals.size() << "\n";
	std::vector<PairWeightLiteral> output_literals = pseudo_boolean_encoder_.HardLessOrEqual(weighted_literals, upper_bound, solver.state_);
	std::cout << "c encoding took " << stopwatch.TimeElapsedInSeconds() << " seconds; " << output_literals.size() << " output literals\n";
	return output_literals;
}

BooleanAssignmentVector UpperBoundSearch::ComputeExtendedSolution(const BooleanAssignmentVector& reference_solution, ConstraintSatisfactionSolver& solver, double time_limit_in_seconds)
{
	std::vector<BooleanLiteral> assumptions(reference_solution.NumVariables());
	for (int i = 0; i < reference_solution.NumVariables(); i++)
	{
		BooleanVariable variable(i + 1);
		assumptions[i] = BooleanLiteral(variable, reference_solution[variable]);
	}
	SolverOutput output = solver.Solve(assumptions, time_limit_in_seconds);
	runtime_assert(output.HasSolution()); //although it could be that the solver timeout, for now we consider this strange
	return output.solution;
}

bool UpperBoundSearch::StrengthenUpperBound(const std::vector<PairWeightLiteral>& sum_literals, int64_t upper_bound, ConstraintSatisfactionSolver& solver)
{
//	if (upper_bound_propagator_ != NULL)
//	{
//		bool success = upper_bound_propagator_->StrengthenUpperBound(solver.state_, upper_bound);
//		return success;
//	}
        if (upper_bound_constraint_ != nullptr) {
          bool success = upper_bound_constraint_->UpdateMax(upper_bound, solver.state_);
          return success;
        }

	//todo we may be trying to readd unit clauses from previous iterations; fix
	bool added_unit_clause = false;
	for (auto iter = sum_literals.rbegin(); iter != sum_literals.rend(); ++iter)
	{
		if (iter->weight > upper_bound)
		{
			added_unit_clause = true;
			bool success = solver.state_.AddUnitClause(~iter->literal);
			if (!success) { return false; }
		}
	}
	runtime_assert(added_unit_clause);
	return true;
}

void UpperBoundSearch::SetValueSelectorValues(ConstraintSatisfactionSolver& solver, LinearBooleanFunction& objective_function, const BooleanAssignmentVector& solution)
{
	switch (value_selection_strategy_)
	{
		case Pumpkin::UpperBoundSearch::ValueSelectionStrategy::PHASE_SAVING:
		{//do nothing
			return;
		}
		case Pumpkin::UpperBoundSearch::ValueSelectionStrategy::SOLUTION_GUIDED_SEARCH:
		{//set the values according to the best solution
			solver.state_.value_selector_.SetPhaseValuesAndFreeze(solution);
			return;
		}
		case Pumpkin::UpperBoundSearch::ValueSelectionStrategy::OPTIMISTIC:
		{//set the values according to the best solution, apart from the objective literals, which are set to optimistic zero values
			BooleanAssignmentVector modified_solution(solution);
			for (BooleanLiteral objective_literal : objective_function)
			{
				modified_solution.AssignLiteral(objective_literal, false);
			}
			solver.state_.value_selector_.SetPhaseValuesAndFreeze(modified_solution);
			return;
		}
		case Pumpkin::UpperBoundSearch::ValueSelectionStrategy::OPTIMISTIC_AUX:
		{//similar as above, but assigns zero to all (auxiliary) variables used in the generalised totaliser encoding for the upper bound
			BooleanAssignmentVector modified_solution(solution);
			//in some cases the solution might have less variables than currently in the solver, e.g., after core-guided phase or after adding the upper bound constraint
			while (solver.state_.GetNumberOfVariables() > modified_solution.NumVariables()) { modified_solution.Grow(); }

			for (BooleanLiteral objective_literal : objective_function) //I think these could be removed since these variables are included in the next loop todo;
			{
				modified_solution.AssignLiteral(objective_literal, false);
			}
			for (std::vector<PairWeightLiteral>& literals : pseudo_boolean_encoder_.literals_from_last_hard_call_)
			{
				for (PairWeightLiteral& p : literals)
				{
					modified_solution.AssignLiteral(p.literal, false);
				}
			}		
			solver.state_.value_selector_.SetPhaseValuesAndFreeze(modified_solution);			
			return;
		}
		default:
		{
			runtime_assert(1 == 2);
		}
	}
}

int64_t UpperBoundSearch::GetInitialDivisionCoefficient(LinearBooleanFunction& objective_function)
{
	switch (varying_resolution_strategy_)
	{
		case VaryingResolutionStrategy::OFF:
		{
			return 1;
		}
		case VaryingResolutionStrategy::BASIC:
		{
			return std::max(int64_t(1), objective_function.GetMaximumWeight());
		}
		case VaryingResolutionStrategy::RATIO:
		{

			int64_t large_weight = pow(10, 1 + ceil(log10(objective_function.GetMaximumWeight())));
			return std::max(int64_t(1), GetNextDivisionCoefficientRatioStrategy(large_weight, objective_function));
		}
		default:
		{
			runtime_assert(1 == 2); return -1;
		}
	}	
}

int64_t UpperBoundSearch::GetNextDivisionCoefficient(int64_t current_division_coefficient, const LinearBooleanFunction& objective_function)
{
	switch (varying_resolution_strategy_)
	{
		case VaryingResolutionStrategy::OFF:
			return 0;

		case VaryingResolutionStrategy::BASIC:
			return objective_function.GetMaximumWeightSmallerThanThreshold(current_division_coefficient);

		case VaryingResolutionStrategy::RATIO:
			return GetNextDivisionCoefficientRatioStrategy(current_division_coefficient, objective_function);

		default:
			runtime_assert(1 == 2); return -1;
	}
}

LinearBooleanFunction UpperBoundSearch::GetVaryingResolutionObjective(int64_t division_coefficient, LinearBooleanFunction& original_objective)
{
	LinearBooleanFunction simplified_objective;
	for (BooleanLiteral literal : original_objective)
	{
		int64_t new_weight = original_objective.GetWeight(literal) / division_coefficient;
		if (new_weight > 0) { simplified_objective.IncreaseWeight(literal, new_weight); }
	}
	runtime_assert(simplified_objective.NumObjectiveLiterals() > 0);
	return simplified_objective;
}

int64_t UpperBoundSearch::GetNextDivisionCoefficientRatioStrategy(int64_t division_coefficient, const LinearBooleanFunction& objective_function)
{
	runtime_assert(division_coefficient == 1 || division_coefficient % 10 == 0);

	//trivial case, if there are no literals in the objective, return zero to signal nothing needs to be done
	if (objective_function.NumObjectiveLiterals() == 0) { return 0; }
	//if we already tried every weight before, then return zero as the signal to stop
	if (division_coefficient == 1) { return 0; }

	int64_t candidate_coefficient = division_coefficient;
	std::set<int64_t> considered_weights;
	std::vector<BooleanLiteral> considered_literals;
	int64_t previous_number_of_literals_considered = objective_function.GetLiteralsWithWeightGreaterOrEqualToThreshold(division_coefficient).size();
	do
	{		
		candidate_coefficient /= 10;

		considered_literals = objective_function.GetLiteralsWithWeightGreaterOrEqualToThreshold(candidate_coefficient);
		//if we did not add at least one new literals, continue
		if (considered_literals.size() == previous_number_of_literals_considered) { continue; }
		
		for (BooleanLiteral literal : considered_literals) { considered_weights.insert(objective_function.GetWeight(literal)); }
		
		//if we hit the ratio, we found the new coefficient, otherwise reiterate
		if (double(considered_literals.size()) / considered_weights.size() >= 1.25)
		{
			return candidate_coefficient;
		}
	} while (candidate_coefficient >= 10 && considered_literals.size() != objective_function.NumObjectiveLiterals());

	return candidate_coefficient;
}
UpperBoundSearch::~UpperBoundSearch() {
  delete upper_bound_constraint_;
  upper_bound_constraint_ = nullptr;
}

}
