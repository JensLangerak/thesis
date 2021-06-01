#pragma once

#include "boolean_literal.h"
#include "boolean_assignment_vector.h"
#include "pair_weight_literal.h"
#include "../Propagators/Dynamic/Encoders/i_encoder.h"

#include <vector>
#include <string>

namespace Pumpkin
{

struct WeightedLiteral {
  WeightedLiteral() : weight(-1){};
  WeightedLiteral(BooleanLiteral literal, int64_t weight)
      : literal(literal), weight(weight) {}

  BooleanLiteral literal;
  int64_t weight;
};

struct PseudoBooleanConstraint
{
	PseudoBooleanConstraint(std::vector<BooleanLiteral>& lits, std::vector<uint32_t>& coefs, int rhs,
                                IEncoder<PseudoBooleanConstraint>::IFactory *encoder_factory) :encoder_factory(encoder_factory), literals(lits), coefficients(coefs), right_hand_side(rhs) {}

	std::vector<BooleanLiteral> literals;
	std::vector<uint32_t> coefficients;
	int right_hand_side;

        IEncoder<PseudoBooleanConstraint>::IFactory *encoder_factory;
};

struct CardinalityConstraint
{
  CardinalityConstraint(std::vector<BooleanLiteral> &lits, int min, int max,  IEncoder<CardinalityConstraint>::IFactory *encoder_factory) : encoder_factory(encoder_factory), literals(lits), min(min), max(max) {};
  std::vector<BooleanLiteral> literals;
  int min;
  int max;
  IEncoder<CardinalityConstraint>::IFactory *encoder_factory;
};

struct SumConstraint {
  SumConstraint(std::vector<BooleanLiteral> & inputs, std::vector<BooleanLiteral> & outputs,  IEncoder<SumConstraint>::IFactory *encoder_factory) : input_literals(inputs), output_literals(outputs), encoder_factory(encoder_factory) {};
  std::vector<BooleanLiteral> input_literals;
  std::vector<BooleanLiteral> output_literals;
  IEncoder<SumConstraint>::IFactory *encoder_factory;
};

struct PbSumConstraint {
  PbSumConstraint(std::vector<BooleanLiteral> & inputs, std::vector<uint32_t> & input_coefficients, std::vector<BooleanLiteral> & outputs, std::vector<uint32_t > & output_coefficients, IEncoder<PbSumConstraint>::IFactory *encoder_factory) : input_literals(inputs), input_coefficients(input_coefficients), output_literals(outputs), output_coefficients(output_coefficients), encoder_factory(encoder_factory) {};
  std::vector<BooleanLiteral> input_literals;
  std::vector<uint32_t> input_coefficients;
  std::vector<BooleanLiteral> output_literals;
  std::vector<uint32_t> output_coefficients;
  IEncoder<PbSumConstraint>::IFactory *encoder_factory;
};

class ProblemSpecification
{
public:
	static std::string GetFileType(std::string file_location);
	static ProblemSpecification ReadSATFormula(std::string file_location);
	static ProblemSpecification ReadMaxSATFormula(std::string file_location);
		
	void RemoveObjectiveLiteral(BooleanLiteral target_literal);
	void DecreaseWeightForObjectiveLiteral(BooleanLiteral target_literal, int64_t weight);
	void AddObjectiveLiteral(BooleanLiteral new_objective_literal, int64_t weight);

	void AddUnaryClause(BooleanLiteral literal); //unit clauses that are already present will not be be added
	void AddClause(std::vector<BooleanLiteral>& clause); //a clause that contain both polarities of a variable will not be added (e.g. 1 v -1). Furthermore, unit clauses that are already present will not be be added

	BooleanVariable CreateNewVariable();

	//constraints
	std::vector<BooleanLiteral> unit_clauses_;
	std::vector<std::vector<BooleanLiteral> > clauses_;
	std::vector<PseudoBooleanConstraint> pseudo_boolean_constraints_;
//        std::vector<CardinalityConstraint> cardinality_constraints_;
//        std::vector<SumConstraint> sum_constraints_;
//        std::vector<PbSumConstraint> pb_sum_constraints_;

	//objective function
	std::vector<PairWeightLiteral> weighted_literals_;

	int64_t num_Boolean_variables_;
	int64_t num_binary_clauses_;
	int64_t num_ternary_clauses_;
	int64_t num_other_clauses_;
	int64_t num_literals_in_other_clauses_;
	
	bool IsSatisfyingAssignment(const BooleanAssignmentVector& solution); //solution[i] is the truth assignment for the variable with index i
	int ComputeCost(const BooleanAssignmentVector& solution);
	int64_t GetLiteralWeight(BooleanLiteral literal);
	static bool EvaluateClause(std::vector<BooleanLiteral>& clause, const BooleanAssignmentVector& solution); //returns whether the clause is satisfied by the given solution
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