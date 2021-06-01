#include "clause_database.h"
#include "watch_list.h"
#include "../../Utilities/boolean_literal.h"
#include "../../Engine/solver_state.h"

#include <algorithm>
#include <assert.h>
#include <set>
#include <iostream>

namespace Pumpkin
{

ClauseDatabase::ClauseDatabase(
	int64_t num_variables, 
	double decay_factor, 
	int lbd_threshold, 
	int target_max_num_temporary_clauses,
	bool use_LBD_for_sorting_temporary_clauses
):
	watch_list_(num_variables),
	increment_(1.0),
	max_threshold_(1e100),
	decay_factor_(decay_factor),
	LBD_threshold_(lbd_threshold),
	target_max_num_temporary_clauses_(target_max_num_temporary_clauses),
	use_LBD_for_sorting_temporary_clauses_(use_LBD_for_sorting_temporary_clauses),
	counter_total_removed_clauses_(0),
	number_of_learned_literals_(0),
	number_of_learned_clauses_(0)
{
}

TwoWatchedClause * ClauseDatabase::AddPermanentClause(std::vector<BooleanLiteral> &literals, const SolverState &state)
{
	TwoWatchedClause *added_clause = TwoWatchedClause::MakeTwoWatchedClause(literals, false, state);
	permanent_clauses_.push_back(added_clause);	
	AddWatchers(added_clause);
	return added_clause;
}

TwoWatchedClause * ClauseDatabase::AddLearnedClause(std::vector<BooleanLiteral> &literals, const SolverState &state)
{
	TwoWatchedClause *added_clause = TwoWatchedClause::MakeTwoWatchedClause(literals, true, state);
	if (added_clause->best_literal_blocking_distance_ <= LBD_threshold_)
	{
		learned_clauses_.low_lbd_clauses.push_back(added_clause);
	}
	else
	{
		learned_clauses_.temporary_clauses.push_back(added_clause);
	}
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
	//binary clauses are prioritised - todo probably not needed given that the lbd is low for binary clauses
	if (c1->IsBinary() == true && c2->IsBinary() == false) return true;
	if (c1->IsBinary() == false && c2->IsBinary() == true) return false;

	//compare based on the lbd score computed during the search (could be different now but its not recomputed!)
	if (c1->best_literal_blocking_distance_ < c2->best_literal_blocking_distance_) return true;
	if (c1->best_literal_blocking_distance_ > c2->best_literal_blocking_distance_) return false;

	//at this point both or neither clauses are binary and they have the same lbd score
	assert((c1->IsBinary() && c2->IsBinary() || c1->Size() > 2 && c2->Size() > 2) && (c1->best_literal_blocking_distance_ == c2->best_literal_blocking_distance_));

	//the standard minisat comparison based on activity
	return c1->activity_ > c2->activity_;
}

void ClauseDatabase::PromoteAndReduceTemporaryClauses()
{
	//for simplicity of implementation, we first promote clauses, and then we see which ones to remove
	//	we could do it in one loop, probably does not make a big difference

	//first promote clauses if their LBD is within the threshold
	int current_index = 0, end_index = 0;
	while (current_index < learned_clauses_.temporary_clauses.size())
	{
		TwoWatchedClause* candidate_clause = learned_clauses_.temporary_clauses[current_index];
		if (candidate_clause->best_literal_blocking_distance_ <= LBD_threshold_)
		{
			learned_clauses_.low_lbd_clauses.push_back(candidate_clause);
		}
		else
		{
			learned_clauses_.temporary_clauses[end_index] = candidate_clause;
			end_index++;
		}
		current_index++;
	}
	learned_clauses_.temporary_clauses.resize(end_index);

	//clauses are sorted in way that 'good' clauses are in the beginning of the array
	if (use_LBD_for_sorting_temporary_clauses_)
	{
		sort(
			learned_clauses_.temporary_clauses.begin(),
			learned_clauses_.temporary_clauses.end(),
			ClauseComparisonCriteria
		);
	}
	else
	{
		sort(
			learned_clauses_.temporary_clauses.begin(),
			learned_clauses_.temporary_clauses.end(),
			[](const TwoWatchedClause* c1, const TwoWatchedClause* c2)->bool
			{
				return c1->activity_ > c2->activity_;
			}
		);
	}	

	//the clauses at the back of the array are the 'bad' clauses
	//we start removals from the end of the array
	int i = int(learned_clauses_.temporary_clauses.size() - 1); //better use signed over unsigned, since decrementing an unsigned zero will overflow
	while (i >= 0 && learned_clauses_.temporary_clauses.size() > target_max_num_temporary_clauses_/2)
	{
		TwoWatchedClause * c = learned_clauses_.temporary_clauses[i];
		//clauses which updated their lbd since last clause clean up are protected from removal for one round
		//	todo not sure if this makes a difference in practice
		if (c->lbd_update_protection_ == true)
		{
			c->lbd_update_protection_ = false;
		}
		//remove
		else
		{			
			RemoveWatchers(c);
			learned_clauses_.temporary_clauses[i] = learned_clauses_.temporary_clauses.back(); //we swap places with the clause at the back, and then pop. Normally we could simply pop the clause, but since we may protect clauses in the previous if statement, we need to do it like this
			learned_clauses_.temporary_clauses.pop_back();
			delete c;

			counter_total_removed_clauses_++;			
		}
		i--;
	}
}

void ClauseDatabase::RemoveAllLearnedClauses()
{
	for(int i = 0; i < learned_clauses_.low_lbd_clauses.size(); i++)
	{
		TwoWatchedClause* c = learned_clauses_.low_lbd_clauses[i];
		RemoveWatchers(c);
		delete c;
		counter_total_removed_clauses_++;
	}

	for (int i = 0; i < learned_clauses_.temporary_clauses.size(); i++)
	{
		TwoWatchedClause* c = learned_clauses_.temporary_clauses[i];
		RemoveWatchers(c);
		delete c;
		counter_total_removed_clauses_++;
	}

	learned_clauses_.low_lbd_clauses.clear();
	learned_clauses_.temporary_clauses.clear();
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

int ClauseDatabase::NumLearnedClauses() const
{
	return int(learned_clauses_.low_lbd_clauses.size() + learned_clauses_.temporary_clauses.size());
}

int ClauseDatabase::NumClausesTotal() const
{
	return int(permanent_clauses_.size()) + NumLearnedClauses();
}

int ClauseDatabase::NumTemporaryClauses() const
{
	return learned_clauses_.temporary_clauses.size();
}

bool ClauseDatabase::TemporaryClausesExceedLimit() const
{
	return NumTemporaryClauses() >= target_max_num_temporary_clauses_;
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
	for (TwoWatchedClause * c : learned_clauses_.temporary_clauses) { c->activity_ /= max_threshold_; }
	increment_ /= max_threshold_;
}

//debug methods below-------------------------------

bool ClauseDatabase::AreWatchesCorrect(const SolverState &state) const
{
	if (AreClausesProperlyWatched(permanent_clauses_, state) == false) return false;
	if (AreClausesProperlyWatched(learned_clauses_.low_lbd_clauses, state) == false) return false;
	if (AreClausesProperlyWatched(learned_clauses_.temporary_clauses, state) == false) return false;
	return true;
}

bool ClauseDatabase::NothingToPropagate(const SolverState &state) const
{
	for (TwoWatchedClause * c : permanent_clauses_){ assert(c->ShouldPropagate(state) == false); }
	for (TwoWatchedClause * c : learned_clauses_.low_lbd_clauses){ assert(c->ShouldPropagate(state) == false); }
	for (TwoWatchedClause* c : learned_clauses_.temporary_clauses){ assert(c->ShouldPropagate(state) == false); }
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

bool ClauseDatabase::IsClauseProperlyWatched(const TwoWatchedClause *c, const SolverState &state) const
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

void ClauseDatabase::RecomputeAndPrintClauseLengthStatsForPermanentClauses(bool print_clauses)
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

		if (print_clauses)
		{
			for (int i = 0; i < clause->literals_.Size(); i++)
			{
				BooleanLiteral lit = clause->literals_.operator[](i);
				if (lit.IsPositive())
				{
					std::cout << lit.VariableIndex() << " ";
				}
				else
				{
					std::cout << "-" << lit.VariableIndex() << " ";
				}				
			}
			std::cout << "\n";
		}

		for (BooleanLiteral literal : clause->literals_)
		{
			variable_indicies.insert(literal.VariableIndex());
		}
	}

	std::cout << "c Effective number of variables: " << variable_indicies.size() << "\n";
	std::cout << "c Number of clauses: " << number_of_clauses << "\n";
	std::cout << "c Number of literals: " << number_of_literals << "\n";
	std::cout << "c \tbinary clauses: " << number_of_binary_clauses << "\n";
	std::cout << "c \tternary clauses: " << number_of_ternary_clauses << "\n";
	std::cout << "c \tother clauses: " << number_of_other_clauses << "\n";
	std::cout << "c \tAvg size of other clauses: " << double(sum_of_lengths_of_other_clauses)/number_of_other_clauses << "\n";
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
  RemoveAllLearnedClauses();
  for (auto c : permanent_clauses_) {
    delete c;
  }
  permanent_clauses_.clear();
}

} //end Pumpkin namespace