#include "watch_list_pseudo_boolean.h"

namespace Pumpkin
{

WatchListPseudoBoolean::WatchListPseudoBoolean(int num_variables)
	:watch_lists_(2 * num_variables + 2) //two spots for each variable (positive and negative literal), and the bottom two spots are not used
{
}

void WatchListPseudoBoolean::Add(BooleanLiteral literal, WatchedPseudoBooleanConstraint * constraint)
{
	int literal_index = ComputeIndexForLiteral(literal);
	watch_lists_[literal_index].push_back(WatcherPseudoBoolean(constraint));
}

void WatchListPseudoBoolean::Remove(BooleanLiteral literal, WatchedPseudoBooleanConstraint * constraint)
{
	int literal_index = ComputeIndexForLiteral(literal);
	for (int clause_index = 0; clause_index < watch_lists_[literal_index].size(); clause_index++)
	{
		if (watch_lists_[literal_index][clause_index].constraint_ == constraint)
		{
			watch_lists_[literal_index][clause_index] = watch_lists_[literal_index].back();
			watch_lists_[literal_index].pop_back();
			return;
		}
	}
	assert(1 == 2); //something must have been removed, should never reach this
}

void WatchListPseudoBoolean::Remove(BooleanLiteral literal, int constraint_index)
{
	int literal_index = ComputeIndexForLiteral(literal);
	watch_lists_[literal_index][constraint_index] = watch_lists_[literal_index].back();
	watch_lists_[literal_index].pop_back();
}

void WatchListPseudoBoolean::ClearWatches(BooleanLiteral literal)
{
	int literal_index = ComputeIndexForLiteral(literal);
	watch_lists_[literal_index].clear();
}

void WatchListPseudoBoolean::Grow()
{
	watch_lists_.push_back(std::vector<WatcherPseudoBoolean>());
	watch_lists_.push_back(std::vector<WatcherPseudoBoolean>());
}

const std::vector<WatcherPseudoBoolean>& WatchListPseudoBoolean::operator[](BooleanLiteral literal) const
{
	int literal_index = ComputeIndexForLiteral(literal);
	return watch_lists_[literal_index];
}

std::vector<WatcherPseudoBoolean>& WatchListPseudoBoolean::operator[](BooleanLiteral literal)
{
	int literal_index = ComputeIndexForLiteral(literal);
	return watch_lists_[literal_index];
}

bool WatchListPseudoBoolean::IsLiteralWatchingConstraint(BooleanLiteral literal, const WatchedPseudoBooleanConstraint * constraint) const
{
	std::vector<WatcherPseudoBoolean> watched_clauses = operator[](literal);
	//return std::find(watched_clauses.begin(), watched_clauses.end(), clause) != watched_clauses.end();
	for (WatcherPseudoBoolean &w : watched_clauses)
	{
		if (w.constraint_ == constraint)
		{
			return true;
		}
	}
	return false;
}

int WatchListPseudoBoolean::ComputeIndexForLiteral(BooleanLiteral literal) const
{
	return 2 * literal.Variable().index_ + literal.IsNegative();
}

} //end Pumpkin namespace