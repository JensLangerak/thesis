#include "database_clausal_binary.h"

namespace Pumpkin
{
/*DatabaseClausalBinary::DatabaseClausalBinary(int num_variables) :
binary_clauses_(2 * num_variables + 2), //two spots for each variable (positive and negative literal), and the bottom two spots are not used
num_binary_clauses_(0)
{
}

void DatabaseClausalBinary::AddClause(BooleanLiteral literal1, BooleanLiteral literal2)
{
	num_binary_clauses_++;
	binary_clauses_[literal1].push_back(literal2);
	binary_clauses_[literal2].push_back(literal1);
}

void DatabaseClausalBinary::Grow()
{
	//need to push two elements, one for each polarity
	binary_clauses_.push_back(std::vector<BooleanLiteral>());
	binary_clauses_.push_back(std::vector<BooleanLiteral>());
}

int64_t DatabaseClausalBinary::Size() const
{
	return num_binary_clauses_;
}

bool DatabaseClausalBinary::Empty() const
{
	return Size() == 0;
}

bool DatabaseClausalBinary::IsBinaryClausePresent(BooleanLiteral literal1, BooleanLiteral literal2)
{
	//recall that the binary watch list is symmetric, i.e., if (a v b) is a clause, then 'a' watches 'b' and 'b' watches 'a'
	
	//let the first literal be the one with the smaller watch list
	if (binary_clauses_[literal1].size() > binary_clauses_[literal2].size()) { std::swap(literal1, literal2); }

	for (BooleanLiteral lit : binary_clauses_[literal1])
	{
		if (lit == literal2) { return true; }
	}
	return false;
}

const std::vector<BooleanLiteral>& DatabaseClausalBinary::operator[](BooleanLiteral literal) const
{
	return binary_clauses_[literal];
}*/

}//end namespace Pumpkin