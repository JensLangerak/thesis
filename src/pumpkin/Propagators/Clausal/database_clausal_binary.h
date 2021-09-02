#pragma once

/*#include "..\..\Utilities\boolean_literal.h"
#include "..\..\Utilities\vector_object_indexed.h"

namespace Pumpkin
{
class DatabaseClausalBinary
{
public:
	DatabaseClausalBinary(int num_variables);//create a watch list for 'num_variables' variables

	void AddClause(BooleanLiteral literal1, BooleanLiteral literal2); //Adds the consisting of two input literals.
	void Grow(); //increases the number of watched variables by one. The newly added variable does not watch any clauses.
	
	int64_t Size() const;
	bool Empty() const;
	bool IsBinaryClausePresent(BooleanLiteral literal1, BooleanLiteral literal2);

	const std::vector<BooleanLiteral>& operator[](BooleanLiteral literal) const; //returns the reference to the vector of BooleanLiteral that represents binary clauses that include 'literal', e.g., [lit1] = {lit2, lit3} means that the clauses (lit1, lit2) and (lit1, lit3) are stored
	
private:
	VectorObjectIndexed<BooleanLiteral, std::vector<BooleanLiteral> > binary_clauses_;
	int64_t num_binary_clauses_;
};
} //end Pumpkin namespace*/