#include "constraint_optimisation_solver.h"
#include "preprocessor.h"
#include "../Utilities/runtime_assert.h"
#include "../Utilities/solver_output_checker.h"
#include "../Utilities/stopwatch.h"
#include "../Utilities/boolean_assignment_vector.h"
#include "../Utilities/linear_boolean_function.h"
#include "../Utilities/gz_file_reader.h"
#include <zlib.h>
#include "../Utilities/directly_hashed_boolean_literal_set.h"

#include <algorithm>
#include <iostream>

namespace Pumpkin
{
/*ConstraintOptimisationSolver::ConstraintOptimisationSolver(ParameterHandler& parameters) :
	constrained_satisfaction_solver_(parameters),
	//original_problem_specification_(problem_specification),
	core_guided_searcher_(parameters),
	linear_searcher_(parameters)
{
	parameters.PrintParametersDifferentFromDefault();
	//read benchmark into solver...

}*/

/*ConstraintOptimisationSolver::ConstraintOptimisationSolver(ProblemSpecification& problem_specification, ParameterHandler& parameters) :
	constrained_satisfaction_solver_(problem_specification, parameters),
	original_problem_specification_(problem_specification),
	core_guided_searcher_(parameters),
	linear_searcher_(parameters)
{
	parameters.PrintParametersDifferentFromDefault();
}*/

ConstraintOptimisationSolver::ConstraintOptimisationSolver(ProblemSpecification* problem_specification, ParameterHandler& parameters):
    constrained_satisfaction_solver_(problem_specification, parameters),
    core_guided_searcher_(parameters),
    linear_searcher_(parameters),
    use_lexicographical_objectives_(parameters.GetBooleanParameter("lexicographical")),
    optimistic_initial_solution_(parameters.GetBooleanParameter("optimistic-initial-solution")) {
  for (auto wl : problem_specification->weighted_literals_) {
    original_objective_function_.IncreaseWeight(wl.literal, wl.weight);
  }
  constrained_satisfaction_solver_.state_.variable_selector_.Reset(parameters.GetIntegerParameter("seed"));

}

ConstraintOptimisationSolver::ConstraintOptimisationSolver(ParameterHandler& parameters):
	constrained_satisfaction_solver_(parameters),
	core_guided_searcher_(parameters),
	linear_searcher_(parameters),
	use_lexicographical_objectives_(parameters.GetBooleanParameter("lexicographical")),
	optimistic_initial_solution_(parameters.GetBooleanParameter("optimistic-initial-solution"))
{
	parameters.PrintParametersDifferentFromDefault();

	//read instance into the solver

	clock_t start = clock();
	GzFileReader file_reader(parameters.GetStringParameter("file"));

	char c;
	//skip comments
	file_reader >> c;
	while (c == 'c') 
	{ 
		file_reader.SkipLine(); 
		file_reader >> c;
	}
	runtime_assert(c == 'p');

	int64_t num_clauses, num_variables, number;
	std::string s;
	std::vector<BooleanLiteral> clause;
	DirectlyHashedBooleanLiteralSet seen_literals;
	BooleanLiteral literal;
	//read the header
	file_reader >> s;
	//SAT file format
	if (s == "cnf")
	{
		std::cout << "c reading SAT file\n";
		file_reader >> num_variables >> num_clauses;
		constrained_satisfaction_solver_.state_.CreateVariablesUpToIndex(abs(num_variables));
		seen_literals.Insert(BooleanLiteral(BooleanVariable(num_variables), true));
		seen_literals.Clear();

		//clauses are read one after the other; each iteration is one clause
		while (file_reader.IsOk())
		{
			clause.clear();
			seen_literals.Clear();
			//each clause has at least one literal; push the first literal into the clause
			file_reader >> number;
			runtime_assert(number != 0);
			runtime_assert(abs(number) <= num_variables);
			literal = BooleanLiteral(BooleanVariable(abs(number)), number > 0);
			clause.push_back(literal);
			seen_literals.Insert(literal);
			//find the remaining literals
			bool is_trivially_redundant = false; //a clause is considered trivially redundant if it contains both polarities of a single variable, e.g., (x v ~x)
			file_reader >> number;
			while (number != 0)
			{
				runtime_assert(abs(number) <= num_variables);
				literal = BooleanLiteral(BooleanVariable(abs(number)), number > 0);
				
				if (seen_literals.IsPresent(~literal)) //check if the other polarity is already in the clause
				{
					is_trivially_redundant = true;
					file_reader.SkipLine();
					break;
				}
				//only add the literal if it is not present in the clause already; some benchmarks might repeat the same literal multiple times					
				if (!seen_literals.IsPresent(literal)) 
				{ 
					seen_literals.Insert(literal);
					clause.push_back(literal); 
				}
				file_reader >> number;
			}

			if (is_trivially_redundant) 
			{ 
				/*do nothing; no need to add trivially redundant clauses*/ 
			}
			else if (clause.size() == 1) 
			{ 
				bool success = constrained_satisfaction_solver_.state_.AddUnitClause(clause[0]); 
				runtime_assert(success); //for now we assume the instance is not trivially unsat
			}
			else 
			{ 
				auto conflicting_propagator = constrained_satisfaction_solver_.state_.AddClause(clause); 
				runtime_assert(conflicting_propagator == NULL); 
			}
			file_reader.SkipWhitespaces();
			file_reader.PeekNextChar(c); //if we are at the end of the file, this will set the file reader into a 'not ok' state
		}
		std::cout << "c Time spent reading the file: " << double(clock() - start) / CLOCKS_PER_SEC << "\n";
	}
	//MaxSAT file format
	else if (s == "wcnf")
	{
		std::cout << "c Reading MaxSAT file\n";
		file_reader >> num_variables >> num_clauses;
		constrained_satisfaction_solver_.state_.CreateVariablesUpToIndex(abs(num_variables));
		int64_t hard_clause_weight;
		file_reader >> hard_clause_weight;
		int64_t iter = 0;
		std::cout << "c num_vars: " << num_variables << "; num_clauses: " << num_clauses << "\n";
		seen_literals.Insert(BooleanLiteral(BooleanVariable(num_variables), true));
		seen_literals.Clear();

		//clauses are read one after the other; each iteration is one clause
		while (file_reader.IsOk())
		{
			//if (iter % 10000 == 0) { std::cout << "c\t" << iter << "\n"; }

			iter++;
			clause.clear();
			seen_literals.Clear();
			file_reader >> number;
			//std::cout << number << " ";
			if (number == hard_clause_weight)
			{
				//each clause has at least one literal; push the first literal into the clause
				file_reader >> number;
				//std::cout << number << " ";
				runtime_assert(number != 0);
				runtime_assert(abs(number) <= num_variables);
				literal = BooleanLiteral(BooleanVariable(abs(number)), number > 0);
				clause.push_back(literal);
				seen_literals.Insert(literal);
				//find the remaining literals
				bool is_trivially_redundant = false; //a clause is considered trivially redundant if it contains both polarities of a single variable, e.g., (x v ~x)
				file_reader >> number;
				//std::cout << number << " ";
				while (number != 0)
				{
					runtime_assert(abs(number) <= num_variables);
					literal = BooleanLiteral(BooleanVariable(abs(number)), number > 0);

					if (seen_literals.IsPresent(~literal)) //check if the other polarity is already in the clause
					{
						is_trivially_redundant = true;
						file_reader.SkipLine();
						//std::cout << "skipped \n";
						break;
					}
					//only add the literal if it is not present in the clause already; some benchmarks might repeat the same literal multiple times					
					if (!seen_literals.IsPresent(literal)) 
					{ 
						clause.push_back(literal); 
						seen_literals.Insert(literal);
					}
					file_reader >> number;
					//std::cout << number << " ";
				}
				if (is_trivially_redundant)
				{
					/*do nothing; no need to add trivially redundant clauses*/
				}
				else if (clause.size() == 1)
				{
					bool success = constrained_satisfaction_solver_.state_.AddUnitClause(clause[0]);
					runtime_assert(success); //for now we assume the instance is not trivially unsat
				}
				else
				{
					auto conflicting_propagator = constrained_satisfaction_solver_.state_.AddClause(clause);
					runtime_assert(conflicting_propagator == NULL);
				}
				//std::cout << "\n";
				file_reader.SkipWhitespaces();
				file_reader.PeekNextChar(c); //if we are at the end of the file, this will set the file reader into a 'not ok' state
				if (c == '\0') { break; }			
			}
			else //soft clause
			{
				int64_t weight = number;
				clause.push_back(BooleanLiteral::UndefinedLiteral()); //placeholder soft literal
				//each clause has at least one literal; push the first literal into the clause
				file_reader >> number;
				//std::cout << number << " ";
				runtime_assert(number != 0);
				runtime_assert(abs(number) <= num_variables);
				literal = BooleanLiteral(BooleanVariable(abs(number)), number > 0);
				clause.push_back(literal);
				seen_literals.Insert(literal);
				//find the remaining literals
				bool is_trivially_redundant = false; //a clause is considered trivially redundant if it contains both polarities of a single variable, e.g., (x v ~x)
				file_reader >> number;
				//std::cout << number << " ";
				while (number != 0)
				{
					runtime_assert(abs(number) <= num_variables);
					literal = BooleanLiteral(BooleanVariable(abs(number)), number > 0);

					if (seen_literals.IsPresent(~literal)) //check if the other polarity is already in the clause
					{
						is_trivially_redundant = true;
						file_reader.SkipLine();
						//std::cout << "skipped\n";
						break;
					}
					//only add the literal if it is not present in the clause already; some benchmarks might repeat the same literal multiple times					
					if (!seen_literals.IsPresent(literal))
					{
						clause.push_back(literal);
						seen_literals.Insert(literal);
					}
					file_reader >> number;
					//std::cout << number << " ";
				}
				runtime_assert(clause.size() > 1); //since we have a placeholder in the first position, the clause must be larger
				if (is_trivially_redundant)
				{
					/*do nothing; no need to add trivially redundant clauses*/
				}
				else if (clause.size() == 2) //a unit soft clause
				{
					original_objective_function_.IncreaseWeight(~clause[1], weight);
				}
				else
				{
					BooleanVariable fresh_variable = constrained_satisfaction_solver_.state_.CreateNewVariable();
					BooleanLiteral soft_literal(fresh_variable, true);
					clause[0] = soft_literal;
					original_objective_function_.IncreaseWeight(clause[0], weight);
					constrained_satisfaction_solver_.state_.AddClause(clause);
				}
				file_reader.SkipWhitespaces();
				file_reader.PeekNextChar(c); //if we are at the end of the file, this will set the file reader into a 'not ok' state
				
				if (c == '\0') { break; }
			}			
		}
		std::cout << "c Time spent reading the file: " << double(clock() - start) / CLOCKS_PER_SEC << "\n";
	}
	else
	{
		std::cout << "Something wrong with the file; expected cnf or wcnf and got \"" << s << "\" instead!\n";
		exit(1);
	}
	constrained_satisfaction_solver_.state_.variable_selector_.Reset(parameters.GetIntegerParameter("seed"));
}

SolverOutput ConstraintOptimisationSolver::Solve(int64_t time_limit_in_seconds_linear_search, int64_t time_limit_in_seconds_core_guided)
{
	Stopwatch stopwatch(time_limit_in_seconds_linear_search + time_limit_in_seconds_core_guided);
	LinearBooleanFunction objective_function(original_objective_function_);
	solution_tracker_ = SolutionTracker(objective_function, stopwatch);

	bool should_print = constrained_satisfaction_solver_.state_.propagator_clausal_.clause_database_.NumClausesTotal() <= 30;
	constrained_satisfaction_solver_.state_.propagator_clausal_.clause_database_.RecomputeAndPrintClauseLengthStatsForPermanentClauses(should_print);
	
	//find an initial solution by just looking at the hard constraints
	SolverOutput initial_output = constrained_satisfaction_solver_.Solve(stopwatch.TimeLeftInSeconds());
	if (initial_output.ProvenInfeasible()) { return SolverOutput(stopwatch.TimeElapsedInSeconds(), false, BooleanAssignmentVector(), -1, Disjunction()); }
	else if (initial_output.timeout_happened) { return SolverOutput(stopwatch.TimeElapsedInSeconds(), true, BooleanAssignmentVector(), -1, Disjunction()); }
	
	//proceed with the initial solution	
	solution_tracker_.UpdateBestSolution(initial_output.solution);

	core_guided_searcher_.Solve(constrained_satisfaction_solver_, objective_function, solution_tracker_, 0, stopwatch.TimeLeftInSeconds() - time_limit_in_seconds_linear_search);

	linear_searcher_.Solve(constrained_satisfaction_solver_, objective_function, solution_tracker_, 0, stopwatch.TimeLeftInSeconds());

	return SolverOutput(stopwatch.TimeElapsedInSeconds(), !stopwatch.IsWithinTimeLimit(), solution_tracker_.GetBestSolution(), solution_tracker_.UpperBound(), Disjunction());
}

SolverOutput ConstraintOptimisationSolver::SolveBMO(int64_t time_limit_in_seconds_linear_search, int64_t time_limit_in_seconds_core_guided)
{
	Stopwatch stopwatch(time_limit_in_seconds_linear_search + time_limit_in_seconds_core_guided);
	LinearBooleanFunction objective_function(original_objective_function_);
	objective_function.ConvertIntoCanonicalForm();
	solution_tracker_ = SolutionTracker(objective_function, stopwatch);

	bool should_print = constrained_satisfaction_solver_.state_.propagator_clausal_.clause_database_.NumClausesTotal() <= 30;
	constrained_satisfaction_solver_.state_.propagator_clausal_.clause_database_.RecomputeAndPrintClauseLengthStatsForPermanentClauses(should_print);

	SolverOutput initial_output = ComputeInitialSolution(objective_function, stopwatch);

	if (initial_output.ProvenInfeasible()) { return SolverOutput(stopwatch.TimeElapsedInSeconds(), false, BooleanAssignmentVector(), -1, Disjunction()); }
	else if (initial_output.timeout_happened) { return SolverOutput(stopwatch.TimeElapsedInSeconds(), true, BooleanAssignmentVector(), -1, Disjunction()); }

	//proceed with the initial solution	
	solution_tracker_.UpdateBestSolution(initial_output.solution);

	//simple preprocessing
	Preprocessor::RemoveLiteralsWithFixedAssignmentFromObjective(constrained_satisfaction_solver_, objective_function);
	bool success = Preprocessor::FixLiteralsWithLargeWeights(constrained_satisfaction_solver_, objective_function, solution_tracker_.UpperBound());
	runtime_assert(success);

	//the algorithm tries to identify the number of lexicographical optimisation functions
	//if there is a large differences in the weight values, it could be that the user encoded two or more objectives that need to be optimised lexicographically
	//we optimise one objective at a time
	//	(in most cases there is only one objective function)
	int64_t global_lower_bound = objective_function.GetConstantTerm();
	std::vector<WeightInterval> lexicographical_weight_range = ComputeLexicographicalObjectiveWeightRanges(objective_function);
	if (!use_lexicographical_objectives_)
	{
		lexicographical_weight_range.clear();
		lexicographical_weight_range.push_back(WeightInterval(objective_function.GetMinimumWeight(), objective_function.GetMaximumWeight()));
	}
	else
	{
		std::cout << "c num lexicographical objectives: " << lexicographical_weight_range.size() << "\n";
	}
	
	for (auto iter = lexicographical_weight_range.rbegin(); iter != lexicographical_weight_range.rend(); ++iter)
	{
		LinearBooleanFunction current_objective_function;

		for (BooleanLiteral literal : objective_function)
		{
			int64_t weight = objective_function.GetWeight(literal);
			if (iter->min_weight <= weight && weight <= iter->max_weight)
			{
				current_objective_function.AddLiteral(literal, weight);
			}
		}

		if (use_lexicographical_objectives_) { std::cout << "c lexicographically processing weights in range [" << iter->min_weight << ", " << iter->max_weight << "]\n"; }
		bool optimally_solved = core_guided_searcher_.Solve(constrained_satisfaction_solver_, current_objective_function, solution_tracker_, global_lower_bound, stopwatch.TimeLeftInSeconds() - time_limit_in_seconds_linear_search);
	
		if (!stopwatch.IsWithinTimeLimit()) { break; }
		
		if (!optimally_solved)
		{
			optimally_solved = linear_searcher_.Solve(constrained_satisfaction_solver_, current_objective_function, solution_tracker_, global_lower_bound, stopwatch.TimeLeftInSeconds());
			
			if (!stopwatch.IsWithinTimeLimit()) { break; }

			if (optimally_solved) { std::cout << "c\tlinear search proved optimality\n"; }
		}
		else
		{
			//if core guided search solved to optimality, the objective has been rewritten and the solution found satisfies all current objective literals
			//fix their values to make sure they also hold in future iterations
			for (BooleanLiteral literal : current_objective_function)
			{
				bool success = constrained_satisfaction_solver_.state_.AddUnitClause(~literal);
				runtime_assert(success);
			}
			std::cout << "c\tcore-guided proved optimality\n";
			//set the lower bound to this optimal solution
			int64_t lb_increase = current_objective_function.ComputeSolutionCost(solution_tracker_.GetBestSolution()) - current_objective_function.GetConstantTerm();
			current_objective_function.IncreaseConstantTerm(lb_increase);
		}	
		global_lower_bound += current_objective_function.GetConstantTerm();
		std::cout << "c completed LB = " << global_lower_bound << "; UB = " << solution_tracker_.UpperBound() << "\n";
	}
	return SolverOutput(stopwatch.TimeElapsedInSeconds(), !stopwatch.IsWithinTimeLimit(), solution_tracker_.GetBestSolution(), solution_tracker_.UpperBound(), Disjunction());
}

std::string ConstraintOptimisationSolver::GetStatisticsAsString()
{
	std::string stats = constrained_satisfaction_solver_.GetStatisticsAsString();
	stats += "c Primal integral: " + std::to_string(solution_tracker_.ComputePrimalIntegral()) + "\n";
	return stats;
}

SolverOutput ConstraintOptimisationSolver::GetPreemptiveResult()
{
	return SolverOutput(-1, true, solution_tracker_.GetBestSolution(), solution_tracker_.UpperBound(), Disjunction());
}

ParameterHandler ConstraintOptimisationSolver::CreateParameterHandler()
{
	ParameterHandler parameters;

	parameters.DefineNewCategory("General Parameters");
	parameters.DefineNewCategory("Constraint Satisfaction Solver Parameters");
	parameters.DefineNewCategory("Constraint Satisfaction Solver Parameters - Restarts");
	parameters.DefineNewCategory("Linear Search");
	parameters.DefineNewCategory("Core-Guided Search");

	//GENERAL PARAMETERS----------------------------------------

	parameters.DefineStringParameter
	(
		"file",
		"Location of the instance file.",
		"", //default value
		"General Parameters"
	);

	parameters.DefineIntegerParameter
	(
		"time",
		"Time limit given in seconds. This includes the time of the core-guided search (see -time-core-guided).",
		86400, //default value, one day
		"General Parameters",
		0.0 //min_value
	);

	parameters.DefineIntegerParameter
	(
		"time-core-guided",
		"Time limit given in seconds for the core-guided phase. Setting this value equal to -time will effective run a pure core-guided algorithm",
		30, //default value
		"General Parameters",
		0.0 //min_value
	);

	parameters.DefineIntegerParameter
	(
		"seed",
		"The seed dictates the initial variable ordering. The value -1 means the initial ordering is based on the index of variables, and any nonnegative values triggers a random initial ordering.",
		-1, //default value
		"General Parameters",
		-1 //min_value
	);

	parameters.DefineBooleanParameter
	(	
		"lexicographical",
		"Specifies if lexicographical objectives should be detected. Can make a large difference for instances where the magnitudes of the weights are substantial. Detection lexicographical objectives is computationally cheap and usually there is no downside to keeping this on.",
		true,
		"General Parameters"
	);

	parameters.DefineBooleanParameter
	(
		"optimistic-initial-solution",
		"If set, the solver sets objective literals to zero in value selection. TODO",
		true,
		"General Parameters"
	);

	//CONSTRAINT SATISFACTION SOLVER PARAMETERS-----------------------------------

	parameters.DefineBooleanParameter
	(
		"binary-clause-propagator",
		"Specifies if a specialised propagator should be used for the binary clauses. In this case binary clauses are propagated before other propagations.",
		false, //default value; for now the binary propagator is disabled since it does not support removing clauses, need to add that
		"Constraint Satisfaction Solver Parameters"
	);

	parameters.DefineBooleanParameter
	(
		"clause-minimisation",
		"Use clause minimisation for each learned clause based on [todo insert paper].",
		true, //default value
		"Constraint Satisfaction Solver Parameters"
	);

	parameters.DefineBooleanParameter
	(
		"bump-decision-variables",
		"Additionally increase VSIDS activities for decision variables. Experimental parameter, may be removed later.",
		true, //default value
		"Constraint Satisfaction Solver Parameters"
	);

	parameters.DefineFloatParameter
	(
		"decay-factor-variables",
		"Influences how much weight the solver gives to variable recently involved in conflicts. A high value put emphasise on recent conflicts.",
		0.95, //default
		"Constraint Satisfaction Solver Parameters",
		0.0, //min value
		1.0 //max_value
	);

	parameters.DefineFloatParameter
	(
		"decay-factor-learned-clause",
		"Influences how much weight the solver gives to recent conflicts when deciding on clause removal. A high value put emphasise on recent conflicts.",
		0.99, //default
		"Constraint Satisfaction Solver Parameters",
		0.0, //min value
		1.0 //max_value
	);
	
	parameters.DefineIntegerParameter
	(
		"lbd-threshold",
		"Learned clauses with an LBD value lower or equal to the lbd-threshold are kept forever in the solver. TODO paper",
		5, //default value
		"Constraint Satisfaction Solver Parameters",
		0
	);

	parameters.DefineIntegerParameter
	(
		"limit-num-temporary-clauses",
		"The solver aims at not storing more temporary clauses than the limit. TODO paper.",
		20000, //default value
		"Constraint Satisfaction Solver Parameters",
		0
	);

	parameters.DefineBooleanParameter
	(
		"lbd-sorting-temporary-clauses",
		"When removing temporary clauses, they will be removed in order of LBD if this parameter is set, otherwise according to the clause activities.",
		false, //default value
		"Constraint Satisfaction Solver Parameters"
	);

	//CONSTRAINT SATISFACTION SOLVER PARAMETERS - RESTARTS

	parameters.DefineStringParameter
	(
		"restart-strategy",
		"Specifies the restart strategy used by the solver. Todo description.",
		"glucose",
		"Constraint Satisfaction Solver Parameters",
		{ "glucose", "luby", "constant" }
	);

	parameters.DefineIntegerParameter
	(
		"restart-multiplication-coefficient",
		"Multiplies the base number that defines the number of conflicts before restarting. Used with restart-strategy \"luby\" and \"constant\"",
		512,
		"Constraint Satisfaction Solver Parameters",
		1
	);

	parameters.DefineIntegerParameter
	(
		"glucose-queue-lbd-limit",
		"todo",
		50,
		"Constraint Satisfaction Solver Parameters - Restarts",
		0
	);

	parameters.DefineIntegerParameter
	(
		"glucose-queue-reset-limit",
		"todo",
		5000,
		"Constraint Satisfaction Solver Parameters - Restarts",
		0
	);

	parameters.DefineIntegerParameter
	(
		"num-min-conflicts-per-restart",
		"todo",
		50,
		"Constraint Satisfaction Solver Parameters - Restarts",
		0
	);

	//LINEAR SEARCH PARAMETERS---------------------------------------

	parameters.DefineStringParameter
	(
		"varying-resolution",
		"Use the varying resolution heuristic to prioritise large weights [todo insert paper].",
		"ratio",
		"Linear Search",
		{ "off", "basic", "ratio" }
	);

	parameters.DefineStringParameter
	(
		"value-selection",
		"Determines the value that is first assigned to a variable during search.TODO",
		"solution-guided-search",
		"Linear Search",
		{ "phase-saving", "solution-guided-search", "optimistic", "optimistic-aux" }
	);

	parameters.DefineBooleanParameter
	(
		"ub-propagator",
		"If set, the upper bound constraint is handled with a dedicated propagator. Otherwise, the upper bound constraint is encoded using the generalised totaliser encoding.",
		false,
		"Linear Search"
	);

	parameters.DefineBooleanParameter
	(
		"ub-propagator-bump",
		"Instructs the dedicated upper bound propagator to bump the activity of objective literals with respect to their weight (higher-weighted literals get a bigger bump)",
		false,
		"Linear Search"
	);

	//CORE-GUIDED SEARCH PARAMETERS---------------------------------------

	parameters.DefineStringParameter
	(
		"stratification",
		"Use the stratification heuristic to prioritise cores with large weights based on [todo insert paper].",
		"ratio",
		"Core-Guided Search",
		{ "off", "basic", "ratio" }
	);

	parameters.DefineBooleanParameter
	(
		"weight-aware-core-extraction",
		"Try to extract as many cores as possible before reformulating the instance in core-guided search, see [todo insert paper].",
		true,
		"Core-Guided Search"
	);

	parameters.DefineStringParameter
	(
		"cardinality-encoding",
		"Specifies which cardinality constraint encoding to use when processing cores.",
		"totaliser",
		"Core-Guided Search",
		{"totaliser", "cardinality-network"}
	);

	return parameters;
}

void ConstraintOptimisationSolver::CheckCorrectnessOfParameterHandler(ParameterHandler& parameters)
{
	//check if help is needed

	if (parameters.GetStringParameter("file") == "")
	{
		std::cout << "Instance file not specified!\n";
		exit(1);
	}
	//check if the file exists?
}

std::vector<ConstraintOptimisationSolver::WeightInterval> ConstraintOptimisationSolver::ComputeLexicographicalObjectiveWeightRanges(LinearBooleanFunction& objective_function)
{
	if (objective_function.NumObjectiveLiterals() == 0) { return std::vector<WeightInterval>(); }

	std::vector<int64_t> weights;
	for (BooleanLiteral literal : objective_function) { weights.push_back(objective_function.GetWeight(literal)); }
	
	std::sort(weights.begin(), weights.end());

	std::vector<WeightInterval> objectives;
	int64_t lower_weight = weights[0];
	int64_t partial_sum = weights[0];
	for (size_t i = 1; i < weights.size(); i++)
	{
		int64_t current_weight = weights[i];

		if (current_weight > partial_sum)
		{
			objectives.push_back(WeightInterval(lower_weight, weights[i - 1]));
			lower_weight = current_weight;
			partial_sum = 0;
		}
		partial_sum += current_weight;
	}
	objectives.push_back(WeightInterval(lower_weight, weights.back()));
	return objectives;
}

SolverOutput ConstraintOptimisationSolver::ComputeInitialSolution(LinearBooleanFunction& objective_function, Stopwatch& stopwatch)
{
	if (optimistic_initial_solution_)
	{
		for (BooleanLiteral objective_literal : objective_function)
		{
			constrained_satisfaction_solver_.state_.value_selector_.SetAndFreezeValue(~objective_literal);
		}
	}

	//find an initial solution by just looking at the hard constraints
	SolverOutput initial_output = constrained_satisfaction_solver_.Solve(stopwatch.TimeLeftInSeconds());
	
	if (optimistic_initial_solution_) 
	{ 
		constrained_satisfaction_solver_.state_.value_selector_.UnfreezeAll();
	}

	return initial_output;
}

}//end Pumpkin namespace