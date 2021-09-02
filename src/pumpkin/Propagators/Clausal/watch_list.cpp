#include "watch_list.h"

#include <assert.h>

namespace Pumpkin
{

WatchList::WatchList(int num_variables)
	:watch_lists_(2*num_variables+2) //two spots for each variable (positive and negative literal), and the bottom two spots are not used
{
}

void WatchList::AddClauseToWatch(BooleanLiteral literal, TwoWatchedClause * clause)
{
	int literal_index = ComputeIndexForLiteral(literal);
	BooleanLiteral b = clause->GetOtherWatchedLiteral(literal);
	watch_lists_[literal_index].push_back(WatcherClause(clause, b));
}

void WatchList::RemoveClauseFromWatch(BooleanLiteral literal, TwoWatchedClause * clause)
{
	int literal_index = ComputeIndexForLiteral(literal);
	for (int clause_index = 0; clause_index < watch_lists_[literal_index].size(); clause_index++)
	{
		if (watch_lists_[literal_index][clause_index].clause_ == clause)
		{
			watch_lists_[literal_index][clause_index] = watch_lists_[literal_index].back();
			watch_lists_[literal_index].pop_back();
			return;
		}
	}
	assert(1 == 2); //something must have been removed, should never reach this
}

void WatchList::RemoveClauseFromWatch(BooleanLiteral literal, int clause_index)
{
	int literal_index = ComputeIndexForLiteral(literal);
	watch_lists_[literal_index][clause_index] = watch_lists_[literal_index].back();
	watch_lists_[literal_index].pop_back();
}

void WatchList::ClearWatches(BooleanLiteral literal)
{
	int literal_index = ComputeIndexForLiteral(literal);
	watch_lists_[literal_index].clear();
}

void WatchList::Grow()
{
	watch_lists_.push_back(std::vector<WatcherClause>());
	watch_lists_.push_back(std::vector<WatcherClause>());
}

bool WatchList::IsLiteralWatchingClause(BooleanLiteral literal, const TwoWatchedClause * clause) const
{
	std::vector<WatcherClause> watched_clauses = operator[](literal);
	//return std::find(watched_clauses.begin(), watched_clauses.end(), clause) != watched_clauses.end();
	for (WatcherClause &w : watched_clauses)
	{
		if (w.clause_ == clause)
		{
			return true;
		}
	}
	return false;
}

const std::vector<WatcherClause>& WatchList::operator[](BooleanLiteral literal) const
{
	int literal_index = ComputeIndexForLiteral(literal);
	return watch_lists_[literal_index];
}

std::vector<WatcherClause>& WatchList::operator[](BooleanLiteral literal)
{
	int literal_index = ComputeIndexForLiteral(literal);
	return watch_lists_[literal_index];
}

int WatchList::ComputeIndexForLiteral(BooleanLiteral literal) const
{
	return 2*literal.Variable().index_ + literal.IsNegative();
}

} //end Pumpkin namespace