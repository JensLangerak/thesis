#include "clause_database.h"
#include "watch_list.h"
#include "../../Basic Data Structures/boolean_literal.h"
#include "../../Engine/solver_state.h"

#include <algorithm>
#include <assert.h>
#include <set>
#include <iostream>

namespace Pumpkin
{

ClauseDatabase::ClauseDatabase(int64_t num_variables, double decay_factor):
	watch_list_(num_variables),
	increment_(1.0),
	max_threshold_(1e100),
	decay_factor_(decay_factor),
	counter_total_removed_clauses_(0),
	number_of_learned_literals_(0),
	number_of_learned_clauses_(0)
{
}

/*ClauseDatabase::ClauseDatabase(SATformula &sat_formula):
	watch_list_(sat_formula.num_variables_),
	unit_clauses_(sat_formula.unit_clauses_),
	increment_(1.0),
	max_threshold_(1e100),
	decay_factor_(0.99),
	counter_total_removed_clauses_(0),
	number_of_learned_literals_(0),
	number_of_learned_clauses_(0)
{
	for (Disjunction& literals : sat_formula.clauses_)
	{
		TwoWatchedClause* added_clause = TwoWatchedClause::MakeTwoWatchedClause(literals);// new TwoWatchedClause(literals);
		permanent_clauses_.push_back(added_clause);
		AddWatchers(added_clause);
	}
}*/

TwoWatchedClause * ClauseDatabase::AddPermanentClause(std::vector<BooleanLiteral> &literals, const SolverState &state)
{
	TwoWatchedClause *added_clause = TwoWatchedClause::MakeTwoWatchedClause(literals, false, state);//new TwoWatchedClause(literals, false, state);
	permanent_clauses_.push_back(added_clause);	
	AddWatchers(added_clause);
	return added_clause;
}

TwoWatchedClause * ClauseDatabase::AddTemporaryClause(std::vector<BooleanLiteral> &literals, const SolverState &state)
{
	TwoWatchedClause *added_clause = TwoWatchedClause::MakeTwoWatchedClause(literals, true, state);//new TwoWatchedClause(literals, true, state);
	temporary_clauses_.push_back(added_clause);
	AddWatchers(added_clause);
	number_of_learned_literals_ += literals.size();
	number_of_learned_clauses_++;
	return added_clause;
}

void ClauseDatabase::AddUnitClause(BooleanLiteral &literal)
{
	unit_clauses_.push_back(literal);
}

void ClauseDatabase::AddUnitClauses(std::vector<BooleanLiteral>& units)
{
	for (BooleanLiteral literal : units) { AddUnitClause(literal); }
}

void ClauseDatabase::AddBinaryClause(BooleanLiteral a, BooleanLiteral b, const SolverState &state)
{
	std::vector<BooleanLiteral> lits;
	lits.push_back(a);
	lits.push_back(b);
	AddPermanentClause(lits, state);
}

void ClauseDatabase::AddTernaryClause(BooleanLiteral a, BooleanLiteral b, BooleanLiteral c, const SolverState &state)
{
	std::vector<BooleanLiteral> lits;
	lits.push_back(a);
	lits.push_back(b);
	lits.push_back(c);
	AddPermanentClause(lits, state);
}

void ClauseDatabase::AddWatchers(TwoWatchedClause * clause)
{
	WatchedLiterals watchers(clause->GetWatchedLiterals());
	watch_list_.AddClauseToWatch(watchers.w1, clause);
	watch_list_.AddClauseToWatch(watchers.w2, clause);
}

void ClauseDatabase::RemoveWatchers(TwoWatchedClause * clause)
{
	WatchedLiterals watched_literals = clause->GetWatchedLiterals();	
	watch_list_.RemoveClauseFromWatch(watched_literals.w1, clause);
	watch_list_.RemoveClauseFromWatch(watched_literals.w2, clause);
}

bool ClauseDatabase::ClauseComparisonCriteria(const TwoWatchedClause * c1, const TwoWatchedClause * c2)
{
	//binary clauses are prioritised
	if (c1->IsBinary() == true && c2->IsBinary() == false) return true;
	if (c1->IsBinary() == false && c2->IsBinary() == true) return false;

	//compare based on the lbd score computed during the search (could be different now but its not recomputed!)
	if (c1->best_literal_blocking_distance_ < c2->best_literal_blocking_distance_) return true;
	if (c1->best_literal_blocking_distance_ > c2->best_literal_blocking_distance_) return false;

	//at this point both or neither clauses are binary and they have the same lbd score
	assert((c1->IsBinary() && c2->IsBinary() || c1->Size() > 2 && c2->Size() > 2) && (c1->best_literal_blocking_distance_ == c2->best_literal_blocking_distance_));

	//the standard minisat comparison based on activity
	return c1->activity_ > c2->activity_;
	//sort(temporary_clauses_.begin(), temporary_clauses_.end(), ClauseComparisonCriteria);//[](const TwoWatchedClause * c1, const TwoWatchedClause * c2) -> bool { return c1->activity_ > c2->activity_; });
}

void ClauseDatabase::ReduceTemporaryClauses()
{
	//remove half of the clauses sorted by LBD, then activity
	//this should only be called at the root level though, since some clauses might be used in propagation - will be changed later
	
	//sort is descending order based on Size, lbd, and activity
	sort(temporary_clauses_.begin(), temporary_clauses_.end(), ClauseComparisonCriteria);
	//sort(temporary_clauses_.begin(), temporary_clauses_.end(), [](const TwoWatchedClause *c1, const TwoWatchedClause *c2) { return c1->activity_ > c2->activity_; });

	//was a naive way of testing if it was sorted properly, need to come up with something better
//	assert(temporary_clauses_.Size() == 1 || temporary_clauses_[0]->activity_ >= temporary_clauses_[1]->activity_);

	//std::cout << "Before removal: " << temporary_clauses_.Size() << "\n";
	
	int num_clauses_to_remove = int(temporary_clauses_.size() / 2);
	int counter_removed = 0;
	int i = int(temporary_clauses_.size() - 1);
	while (i >= 0 && counter_removed < num_clauses_to_remove)
	{
		TwoWatchedClause * c = temporary_clauses_[i];
		if (c->best_literal_blocking_distance_ <= 2 || c->Size() <= 2 ) //keep permanently these clauses
		{
			break; //nothing can be removed, stop
		}
		else if (c->lbd_update_protection_ == true)
		{
			//keep
			c->lbd_update_protection_ = false;
		}
		else
		{
			//remove
			RemoveWatchers(c);
			temporary_clauses_[i] = temporary_clauses_.back();
			temporary_clauses_.pop_back();
			counter_removed++;
			counter_total_removed_clauses_++;
			delete c;
		}
		i--;
	}

	//std::cout << "After removal: " << temporary_clauses_.Size() << "\n";

	return;
	
	for (int i = 0; i < num_clauses_to_remove; i++)
	{
		TwoWatchedClause * removed_clause = temporary_clauses_.back();
		temporary_clauses_.pop_back();

		//if the LBD was set to two during search, promote the clause_ to permanent status instead of deleting it
		if (removed_clause->best_literal_blocking_distance_ <= 2 || removed_clause->Size() <= 2)
		{
			permanent_clauses_.push_back(removed_clause);
		}
		else //delete the clause_
		{
			RemoveWatchers(removed_clause);		
			delete removed_clause;
		}
	}
}

void ClauseDatabase::BumpClauseActivity(TwoWatchedClause *clause)
{
	assert(clause->is_learned_ == true);
	//rescale all activities if needed
	if (clause->activity_ + increment_ > max_threshold_) { RescaleClauseActivities(); }
	clause->activity_ += increment_;
	assert(clause->activity_ < INFINITY);
}

void ClauseDatabase::DecayClauseActivities()
{	
	increment_ *= (1.0/decay_factor_);
}

int ClauseDatabase::NumberOfLearnedClauses() const
{
	return int(temporary_clauses_.size());
}

int ClauseDatabase::NumberOfClausesTotal() const
{
	return int(temporary_clauses_.size() + permanent_clauses_.size());
}

TwoWatchedClause * ClauseDatabase::GetPermanentClause(std::vector<BooleanLiteral>& literals)
{
	for (TwoWatchedClause * c : permanent_clauses_)
	{
		if (c->Size() != literals.size())
			continue;

		bool contains_all_literals = true;
		for (BooleanLiteral literal : literals)
		{
			if (std::find(c->literals_.begin(), c->literals_.end(), literal) == c->literals_.end())
			{
				contains_all_literals = false;
				break;
			}

			if (contains_all_literals)
			{
				return c;
			}
		}
	}
	return NULL;
}

void ClauseDatabase::RescaleClauseActivities()
{
	for (TwoWatchedClause * c : temporary_clauses_) { c->activity_ /= max_threshold_; }
	increment_ /= max_threshold_;
}

//debug methods below-------------------------------

bool ClauseDatabase::AreWatchesCorrect(const SolverState &state) const
{
	if (AreClausesProperlyWatched(permanent_clauses_, state) == false) return false;
	if (AreClausesProperlyWatched(temporary_clauses_, state) == false) return false;
	return true;
}

bool ClauseDatabase::NothingToPropagate(const SolverState &state) const
{
	for (TwoWatchedClause * c : permanent_clauses_)
	{
		assert(c->ShouldPropagate(state) == false);
	}
	for (TwoWatchedClause * c : temporary_clauses_)
	{
		if (c->ShouldPropagate(state))
		{
			c->ShouldPropagate(state);
		}
		assert(c->ShouldPropagate(state) == false);
	}
	return true;
}

bool ClauseDatabase::AreClausesProperlyWatched(const std::vector<TwoWatchedClause*> &clauses, const SolverState &state) const
{
	//check for each clause_, if it is being watched by the literals it is watching
	for (TwoWatchedClause * c : clauses)
	{
		WatchedLiterals watchers(c->GetWatchedLiterals());
		assert(watch_list_.IsLiteralWatchingClause(watchers.w1, c) == true);
		assert(watch_list_.IsLiteralWatchingClause(watchers.w2, c) == true);
		bool both_unassigned = state.assignments_.IsAssigned(watchers.w1) == false && state.assignments_.IsAssigned(watchers.w2) == false;
		if (!(both_unassigned || state.assignments_.IsAssignedTrue(watchers.w1) || state.assignments_.IsAssignedTrue(watchers.w2)))
		{
			//this invariant does not hold anymore because of blocking literals in the watchers
			//todo change
			assert(c->IsSatisfied(state) == true || c->HaxPrint(state));//if the invariant holds, the blocking literal must be assigned true - we will just test satisfiability of the clause instead now
			//return false;
		}
	}

	//todo this checks for all variables and not just the relevant ones - could be duplicating work too if I check permanent and then learned clauses using this method....

	//check for each literal, if it is being watched by the clauses it is watching
	for (int var_index = 1; var_index <= state.GetNumberOfVariables(); var_index++) 
	{
		BooleanVariable boolean_variable(var_index);
		BooleanLiteral positive_literal(boolean_variable, true);
		BooleanLiteral negative_literal(boolean_variable, false);
		assert(IsLiteralProperlyWatched(positive_literal, state) == true);
		assert(IsLiteralProperlyWatched(negative_literal, state) == true);
	}
	return true;
}

bool ClauseDatabase::IsClauseProperlyWatched(const TwoWatchedClause * const c, const SolverState &state) const
{
	WatchedLiterals watchers(c->GetWatchedLiterals());
	assert(watch_list_.IsLiteralWatchingClause(watchers.w1, c) == true);
	assert(watch_list_.IsLiteralWatchingClause(watchers.w2, c) == true);
	bool both_unassigned = state.assignments_.IsAssigned(watchers.w1) == false && state.assignments_.IsAssigned(watchers.w2) == false;
	assert(both_unassigned || state.assignments_.IsAssignedTrue(watchers.w1) || state.assignments_.IsAssignedTrue(watchers.w2) || c->HaxPrint(state));
	
	//check for each literal, if it is being watched by the clauses it is watching
	for (BooleanLiteral literal : c->literals_)
	{
		BooleanVariable boolean_variable(literal.Variable());
		BooleanLiteral positive_literal(boolean_variable, true);
		BooleanLiteral negative_literal(boolean_variable, false);
		assert(IsLiteralProperlyWatched(positive_literal, state) == true);
		assert(IsLiteralProperlyWatched(negative_literal, state) == true);
	}
	return true;
}

void ClauseDatabase::RecomputeAndPrintClauseLengthStatsForPermanentClauses()
{
	long long number_of_literals = 0;
	long long number_of_clauses = 0;
	long long number_of_binary_clauses = 0;
	long long number_of_ternary_clauses = 0;
	long long number_of_other_clauses = 0;
	long long sum_of_lengths_of_other_clauses = 0;
	std::set<int> variable_indicies;

	for (TwoWatchedClause *clause : permanent_clauses_)
	{
		number_of_literals += clause->literals_.Size();
		number_of_clauses++;
		number_of_binary_clauses += (clause->literals_.Size() == 2);
		number_of_ternary_clauses += (clause->literals_.Size() == 3);
		number_of_other_clauses += (clause->literals_.Size() > 3);
		sum_of_lengths_of_other_clauses += (clause->literals_.Size()>3)*clause->literals_.Size();

		for (BooleanLiteral literal : clause->literals_)
		{
			variable_indicies.insert(literal.VariableIndex());
		}
	}

//	std::cout << "c Effective number of variables: " << variable_indicies.size() << "\n";
//	std::cout << "c Number of clauses: " << number_of_clauses << "\n";
//	std::cout << "c Number of literals: " << number_of_literals << "\n";
//	std::cout << "c \tbinary clauses: " << number_of_binary_clauses << "\n";
//	std::cout << "c \tternary clauses: " << number_of_ternary_clauses << "\n";
//	std::cout << "c \tother clauses: " << number_of_other_clauses << "\n";
//	std::cout << "c \tAvg size of other clauses: " << double(sum_of_lengths_of_other_clauses)/number_of_other_clauses << "\n";
}

bool ClauseDatabase::IsLiteralProperlyWatched(BooleanLiteral literal, const SolverState &state) const
{
	const std::vector<WatcherClause> &watched_clauses = watch_list_[literal];
	for (const WatcherClause &w: watched_clauses)
	{
		WatchedLiterals watchers(w.clause_->GetWatchedLiterals());
		assert(watchers.w1 == literal || watchers.w2 == literal);
	}
	return true;
}
ClauseDatabase::~ClauseDatabase() {
  for (auto c : temporary_clauses_)
    delete  c;
//  for (auto  c : permanent_clauses_)
//    delete  c;
}

} //end Pumpkin namespace