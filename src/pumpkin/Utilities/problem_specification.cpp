#include "problem_specification.h"
#include "runtime_assert.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include "../Propagators/Dynamic/Encoders/i_encoder.h"

namespace Pumpkin
{
std::string ProblemSpecification::GetFileType(std::string file_location)
{
	std::ifstream input(file_location.c_str());
	runtime_assert(input);
	std::string s;
	char c;
	int64_t n;

	//skip comments
	while (input.peek() == 'c') { std::getline(input, s); }
	runtime_assert(input.peek() == 'p');

	input >> c;
	runtime_assert(c == 'p');
	input >> s;

	if (s == "wcnf") { return s; }
	else if (s == "cnf") { return s; }
	else { return "unknown"; }
}

ProblemSpecification ProblemSpecification::ReadSATFormula(std::string file_location)
{
	ProblemSpecification problem_specification;

	std::ifstream input(file_location.c_str());
	runtime_assert(input);
	std::string s;
	char c;
	long long n;

	//skip comments
	while (input.peek() == 'c') { std::getline(input, s); }
	runtime_assert(input.peek() == 'p');

	int num_clauses;
	//read the header, i.e. number of variables and clauses
	input >> c;
	runtime_assert(c == 'p');
	input >> s;
	runtime_assert(s == "cnf");

	input >> problem_specification.num_Boolean_variables_ >> num_clauses;

	//read the clauses
	for (int i = 0; i < num_clauses; i++)
	{
		std::vector<BooleanLiteral> clause;
		while (input >> n)
		{
			runtime_assert(n <= problem_specification.num_Boolean_variables_);

			if (n == 0) { break; }; //go until you read a zero, which denotes the end of the clause_

			BooleanVariable var = BooleanVariable(abs(n));
			BooleanLiteral lit = BooleanLiteral(var, n > 0);

			if (std::find(clause.begin(), clause.end(), lit) == clause.end()) //do not push a literal that is already present
			{
				clause.push_back(lit);
			}
		}
		problem_specification.AddClause(clause);			
	}
	return problem_specification;
}

ProblemSpecification ProblemSpecification::ReadMaxSATFormula(std::string file_location)
{
	ProblemSpecification problem_specification;

	std::ifstream input(file_location.c_str());
	runtime_assert(input);
	std::string s;
	char c;
	int64_t n;

	//skip comments
	while (input.peek() == 'c') { std::getline(input, s); }
	runtime_assert(input.peek() == 'p');

	int64_t num_clauses, infinite_weight;
	//read the header, i.e. number of variables and clauses
	input >> c;
	runtime_assert(c == 'p');
	input >> s;
	runtime_assert(s == "wcnf");

	input >> problem_specification.num_Boolean_variables_ >> num_clauses >> infinite_weight;
	BooleanLiteral hard_clause_indicator(BooleanVariable(infinite_weight), true);

	//read the clauses 
	for (int i = 0; i < num_clauses; i++)
	{
		std::vector<BooleanLiteral> clause;
		while (input >> n)
		{
			runtime_assert(clause.empty() || n <= problem_specification.num_Boolean_variables_);

			if (n == 0) { break; }; //go until you read a zero, which denotes the end of the clause_

			BooleanVariable var = BooleanVariable(abs(n));
			BooleanLiteral lit = BooleanLiteral(var, n > 0);

			if (clause.size() == 0 || std::find(clause.begin() + 1, clause.end(), lit) == clause.end()) //do not push literals already present
			{
				clause.push_back(lit);
			}
		}
		runtime_assert(clause.size() >= 2); //recall that clauses must be of size two since the zeros literal is a weight

		if (clause[0] == hard_clause_indicator) //is a hard clause
		{
			clause.erase(clause.begin()); //remove the weight
			problem_specification.AddClause(clause);
		} 
		else //is a soft clause
		{
			//special case: if the soft clause is already unit, we will not create a new objective variable
			if (clause.size() == 2)
			{
				BooleanLiteral objective_literal = ~clause[1];
				int weight = clause[0].Variable().index_; //the index of the 0th variable in a soft clause encodes the weight
				runtime_assert(weight > 0);
				problem_specification.weighted_literals_.push_back(PairWeightLiteral(objective_literal, weight));
			}
			else
			{
				BooleanLiteral objective_literal(problem_specification.CreateNewVariable(), true);
				int weight = clause[0].Variable().index_; //the index of the 0th variable in a soft clause encodes the weight
				runtime_assert(weight > 0);
				clause[0] = objective_literal; //place the objective variable instead of the placeholder (weight)
				problem_specification.weighted_literals_.push_back(PairWeightLiteral(objective_literal, weight));
				problem_specification.AddClause(clause);
			}
		}		
	}
	printf("c done reading the file\n");
	return problem_specification;
}

BooleanVariable ProblemSpecification::CreateNewVariable()
{
	return BooleanVariable(++num_Boolean_variables_);
}

bool ProblemSpecification::IsSatisfyingAssignment(const BooleanAssignmentVector& solution)
{
	runtime_assert(pseudo_boolean_constraints_.empty()); //eventually will include pseudo-Boolean constraints, for now only clauses

	for (BooleanLiteral unit_clause : unit_clauses_)
	{
		if (solution[unit_clause] == false) { return false; }
	}

	for (std::vector<BooleanLiteral>& clause : clauses_)
	{
		if (EvaluateClause(clause, solution) == false) { return false; }
	}
	
	return true;
}

int ProblemSpecification::ComputeCost(const BooleanAssignmentVector& solution)
{
	int64_t cost = 0;
	for (PairWeightLiteral wl : weighted_literals_) { cost += (solution[wl.literal] * wl.weight); }
	return cost;
}

int64_t ProblemSpecification::GetLiteralWeight(BooleanLiteral literal)
{
	for (PairWeightLiteral& wl : weighted_literals_) { if (wl.literal == literal) { return wl.weight; } }
	return 0;
}

bool ProblemSpecification::EvaluateClause(std::vector<BooleanLiteral>& clause, const BooleanAssignmentVector& solution)
{
	for (BooleanLiteral literal : clause)
	{
		if (solution[literal]) { return true; }
	}
	return false;
}

void ProblemSpecification::RemoveObjectiveLiteral(BooleanLiteral target_literal)
{
	for (auto iter = weighted_literals_.begin(); iter != weighted_literals_.end(); ++iter)
	{
		runtime_assert(iter->weight == 1);
		if (iter->literal == target_literal)
		{
			weighted_literals_.erase(iter);
			return;
		}
	}
	runtime_assert(1 == 2);
}

void ProblemSpecification::DecreaseWeightForObjectiveLiteral(BooleanLiteral target_literal, int64_t weight)
{
	for (auto iter = weighted_literals_.begin(); iter != weighted_literals_.end(); ++iter)
	{
		if (iter->literal == target_literal)
		{
			runtime_assert(iter->weight >= weight);
			if (iter->weight == weight) { weighted_literals_.erase(iter); }
			else { iter->weight -= weight; }
			return;
		}
	}
}

void ProblemSpecification::AddObjectiveLiteral(BooleanLiteral new_objective_literal, int64_t weight)
{
	//debug todo remove
	for (auto wl : weighted_literals_) { runtime_assert(wl.literal.Variable() != new_objective_literal.Variable()); }

	weighted_literals_.push_back(PairWeightLiteral(new_objective_literal, weight));
}

void ProblemSpecification::AddUnaryClause(BooleanLiteral literal)
{
	//add the unit clause if it is not already present
	if (std::find(unit_clauses_.begin(), unit_clauses_.end(), literal) == unit_clauses_.end()) 
	{
		unit_clauses_.push_back(literal);
	}
}

void ProblemSpecification::AddClause(std::vector<BooleanLiteral>& clause)
{	
	if (clause.size() == 1) { AddUnaryClause(clause[0]); return; }

	//a clause is trivial if there exists a variable such that literals of both of its polarities are present, e.g., 1 v -1
	bool trivial_clause = false; 
	for (int i = 0; i < clause.size(); i++)
	{
		for (int j = i + 1; j < clause.size(); j++)
		{
			if (clause[i] == ~clause[j])
			{
				trivial_clause = true;
			}
		}
	}

	if (trivial_clause == false)
	{
		clauses_.push_back(clause);
		num_binary_clauses_ += (clause.size() == 2);
		num_ternary_clauses_ += (clause.size() == 3);
		num_other_clauses_ += (clause.size() > 3);
		num_literals_in_other_clauses_ += (clause.size() > 3) * clause.size();
	}
}

ProblemSpecification::ProblemSpecification():
	num_Boolean_variables_(0),
	num_binary_clauses_(0),
	num_ternary_clauses_(0),
	num_other_clauses_(0),
	num_literals_in_other_clauses_(0)
{
}

} //end Pumpkin namespace