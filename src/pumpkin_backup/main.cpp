#include "Engine/constraint_optimisation_solver.h"
#include "Basic Data Structures/solver_parameters.h"

#include <algorithm>
#include <assert.h>
#include <iostream>
#include <vector>
#include <time.h>
#include <cstdlib>
#include <signal.h>
#include <algorithm>
#include <limits>
#include <string>

using namespace std;
using namespace Pumpkin;

ConstraintOptimisationSolver *g_solver;
time_t start_solve;
clock_t start_clock;

void printStatisticsMain()
{
	time_t total_solve = time(0) - start_solve;
	clock_t total_clock = clock() - start_clock;

	g_solver->PrintStats();
	//std::cout << "c conflicts per second (wallclock): " << double(g_solver->counters_.conflicts)/ total_solve << std::endl;
	//std::cout << "c conflicts per second (cpu): " << double(g_solver->counters_.conflicts) / (double(total_clock) / CLOCKS_PER_SEC) << std::endl;
	std::cout << "c wallclock time: " << total_solve << " s " << std::endl;
	std::cout << "c CPU time: " << double(total_clock) / CLOCKS_PER_SEC << std::endl;
	std::cout << "c Primal integral: " << "TODO with new class\n";// g_solver->solution_time_stamps_.ComputePrimalIntegral(total_solve) << std::endl;
}

static void SIGINT_exit(int signum) 
{
	printStatisticsMain();
	std::cout << "s UNKNOWN\n";
	exit(1);
}

void printSolution(vector<bool> &solution)
{
	if (solution.size() == 0)
	{
		cout << "s UNSATISFIABLE\n";
		return;
	}
	
	cout << "s SATISFIABLE\n";

	if (solution.size() >= 100) { return; }

	for (int i = 1; i < solution.size(); i++)
	{
		if (solution[i])
		{
			cout << i << " ";
		}
		else {
			cout << "-" << i << " ";
		}
	}
	cout << endl;
}

void CheckSolutionCorrectness(ProblemSpecification &problem_specification, SolverOutput& output)
{
	if (output.HasSolution())
	{
		bool problem_encountered = false;
		if (problem_specification.IsSatisfyingAssignment(output.solution) == false)
		{
			problem_encountered = true;
			std::cout << "Solution not OK: does not satisfy hard constraints!\n";
		}

		int64_t computed_cost = problem_specification.ComputeCost(output.solution);
		if (computed_cost != output.cost)
		{
			problem_encountered = true;
			std::cout << "Solution not OK: does not match the reported cost!\n";
			std::cout << "\tReported cost: " << output.cost << "\n";
			std::cout << "\tRecomputed cost: " << problem_specification.ComputeCost(output.solution) << "\n";
		}

		if (!problem_encountered) { std::cout << "Solution check OK!\n"; }
	}
}

int main(int argc, char *argv[])
{
	signal(SIGINT, SIGINT_exit);

	string file;
	bool bump_decision_variables = true;
	double time_limit_in_seconds = std::numeric_limits<double>::max();
	
	if (argc > 1)
	{
		file = argv[1]; //will add in the future a class to parse command line parameters
		if (argc > 2) { time_limit_in_seconds = std::stof(argv[2]); }
		if (argc > 3) { bump_decision_variables = false; };
	}
	else 
	{
		//file = "test_instances\\competition\\frb-40-19-1-mgd_c18.cnf";
		//file = "test_instances\\bf0432-007.txt";
		//file = "test_instances\\dubois20.txt";
		//file = "test_instances\\competition\\sv-comp19_prop-reachsafety.sine_8_true-unreach-call_true-termination.i-witness.cnf";
		//file = "test_instances\\competition\\Bibd-sc-10-03-08_c18.cnf";
		//file = "test_instances\\zebra_v155_c1135.txt";
		//file = "test_instances\\competition\\toughsat_23bits_1.cnf";
		//file = "test_instances\\competition\\Eternity-10-05_c18.cnf";
		//file = "test_instances\\engine_4.cnf";
		//file = "test_instances\\testscc2.txt";
		//file = "test_instances\\zebra_v155_c1135.txt";
		//file = "test_instances\\quinn.txt";
		//file = "test_instances\\quinn_maxsat.wcnf";
		file = "test_instances\\xhstt\\BrazilInstance2.xml.wcnf";
		//file = "test_instances\\test_maxsat3.txt";
		//file = "test_instances\\Instance6_11200.txt";
		//file = "test_instances\\test_maxsat3.txt";
		//file = "test_instances\\xhstt\\ItalyInstance4.xml.wcnf";
		//file = "test_instances\\BTBNSL-Rounded_BTWBNSL_Abalone_TWBound_3.wcnf";
		//file = "test_instances\\tcp-tcp_students_105_it_3.wcnf";
		//file = "test_instances\\railway-transport-r11.wcnf";
		//file = "test_instances\\robot-nagivation-robot-navigation_8.wcnf";
	}

	cout << "c File: " << file << endl;

	ProblemSpecification* problem_specification;
	
	if (file.find(".wcnf") != std::string::npos)
	{
		problem_specification = new ProblemSpecification(ProblemSpecification::ReadMaxSATFormula(file));
	}
	else if (file.find(".cnf") != std::string::npos || file.find(".txt") != std::string::npos)
	{
		problem_specification = new ProblemSpecification(ProblemSpecification::ReadSATFormula(file));
	}
	else
	{
		std::cout << "Unknown file format! File ending is expected to be either .cnf or .txt for SAT instances or .wncf for MaxSAT\n";
		std::cout << "File: " << file << "\n";
		return 1;
	}
		
	SolverParameters parameters;
	parameters.bump_decision_variables = bump_decision_variables;
	ConstraintOptimisationSolver solver(*problem_specification, parameters);
	g_solver = &solver;
	start_solve = time(0);
	start_clock = clock();
	SolverOutput solver_output = solver.Solve(time_limit_in_seconds);
	
	CheckSolutionCorrectness(*problem_specification, solver_output);
	
	printStatisticsMain();
	printSolution(solver_output.solution);

	delete problem_specification;
	
    return 0;
}