#pragma once

#include "boolean_literal.h"

#include <vector>
#include <string>

namespace Pumpkin {

struct WeightedLiteral {
  WeightedLiteral() : weight(-1){};
  WeightedLiteral(BooleanLiteral literal, int64_t weight)
      : literal(literal), weight(weight) {}

  BooleanLiteral literal;
  int64_t weight;
};

struct PseudoBooleanConstraint {
  PseudoBooleanConstraint(std::vector<BooleanLiteral> &lits,
                          std::vector<uint32_t> &coefs, int rhs)
      : literals(lits), coefficients(coefs), right_hand_side(rhs) {}

  std::vector<BooleanLiteral> literals;
  std::vector<uint32_t> coefficients;
  int right_hand_side;
};

struct CardinalityConstraint
{
  CardinalityConstraint(std::vector<BooleanLiteral> &lits, int min, int max) : literals(lits), min(min), max(max) {}
  std::vector<BooleanLiteral> literals;
  int min;
  int max;
};

class ProblemSpecification
{
public:
	static ProblemSpecification ReadSATFormula(std::string file_location);
	static ProblemSpecification ReadMaxSATFormula(std::string file_location);
		
	void AddUnaryClause(BooleanLiteral literal); //unit clauses that are already present will not be be added
	void AddClause(std::vector<BooleanLiteral>& clause); //a clause that contain both polarities of a variable will not be added (e.g. 1 v -1). Furthermore, unit clauses that are already present will not be be added

	BooleanVariable CreateNewVariable();

	//constraints
	std::vector<BooleanLiteral> unit_clauses_;
	std::vector<std::vector<BooleanLiteral> > clauses_;
	std::vector<PseudoBooleanConstraint> pseudo_boolean_constraints_;
  std::vector<CardinalityConstraint> dynamic_cardinality_constraints_;
  std::vector<CardinalityConstraint> propagator_cardinality_constraints_;

  //objective function
	std::vector<WeightedLiteral> objective_literals_;

	int64_t num_Boolean_variables_;
	int64_t num_binary_clauses_;
	int64_t num_ternary_clauses_;
	int64_t num_other_clauses_;
	int64_t num_literals_in_other_clauses_;
	
	bool IsSatisfyingAssignment(std::vector<bool>& solution); //solution[i] is the truth assignment for the variable with index i
	int ComputeCost(std::vector<bool>& solution);
	static bool EvaluateLiteral(BooleanLiteral literal, std::vector<bool>& solution); //returns whether the literal evaluates to true in the solution
	static bool EvaluateClause(std::vector<BooleanLiteral>& clause, std::vector<bool>& solution); //returns whether the clause is satisfied by the given solution
																					  /*
	void TriviallyRefineSolution(std::vector<bool> &solution);
	
	void ConvertClausesIntoPseudoBoolean();
	bool ArePseudoBooleanConstraintsSatisfied(std::vector<bool> &solution);
	
	
	void PrintToFile(std::string file_location);
	void AddUnaryClause(BooleanLiteral literal);
	void AddBinaryClause(BooleanLiteral l1, BooleanLiteral l2);
	void AddTernaryClause(BooleanLiteral l1, BooleanLiteral l2, BooleanLiteral l3);
	
	*/

//private:
	ProblemSpecification();
};

} //end Pumpkin namespace