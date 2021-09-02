#include "two_watched_clause.h"
#include "../../Engine/solver_state.h"
#include "../../Utilities/runtime_assert.h"

#include <set>
#include <assert.h>
#include <algorithm>
#include <iostream>

namespace Pumpkin
{

TwoWatchedClause * TwoWatchedClause::MakeTwoWatchedClause(const Disjunction & literals, bool is_learned, const SolverState & state)
{
	assert(literals.size() > 1);
	TwoWatchedClause *clause = static_cast<TwoWatchedClause*>(::operator new(sizeof(TwoWatchedClause) + sizeof(LiteralVector) + literals.size() * sizeof(BooleanLiteral)));
	clause->is_learned_ = is_learned;
	clause->activity_ = 0.0;
	clause->best_literal_blocking_distance_ = int(literals.size());
	clause->lbd_update_protection_ = false;
	clause->locked_in_propagation_ = false;
	clause->literals_.size_ = int(literals.size());
	for (int i = 0; i < int(literals.size()); i++)
	{
		clause->literals_[i] = literals[i];
	}
	
	clause->SetWatches(state);
	clause->updateLBD(state);
	return clause;
}

TwoWatchedClause * TwoWatchedClause::MakeTwoWatchedClause(const Disjunction & literals)
{
	assert(literals.size() > 1);
	TwoWatchedClause *clause = reinterpret_cast<TwoWatchedClause*>(::operator new(sizeof(TwoWatchedClause) + sizeof(LiteralVector) + literals.size() * sizeof(BooleanLiteral)));
	clause->is_learned_ = false;
	clause->activity_ = 0.0;
	clause->best_literal_blocking_distance_ = int(literals.size());
	clause->lbd_update_protection_ = false;
	clause->locked_in_propagation_ = false;

	clause->literals_.size_ = int(literals.size());
	for (int i = 0; i < int(literals.size()); i++)
	{
		clause->literals_[i] = literals[i];
	}
	return clause;
}

//the watched literals are the first two literals
void TwoWatchedClause::SetWatches(const SolverState &state)
{
	auto best_watch = std::min_element(literals_.begin(), literals_.end(), [&state](const BooleanLiteral l1, const BooleanLiteral l2) -> bool { return WatcherComparison(l1, l2, state); });
	int best_position = int(best_watch - literals_.begin());
	std::swap(literals_[0], literals_[best_position]);

	best_watch = std::min_element(literals_.begin() + 1, literals_.end(), [&state](const BooleanLiteral l1, const BooleanLiteral l2) -> bool { return WatcherComparison(l1, l2, state); });
	best_position = int(best_watch - literals_.begin());
	std::swap(literals_[1], literals_[best_position]);
}

bool TwoWatchedClause::ShouldPropagate(const SolverState &state) const
{
	int counter_false_literals = 0, counter_true_literals = 0, counter_unassigned_literals = 0;
	for (int i = 0; i < literals_.Size(); i++)
	{
		if (state.assignments_.IsAssignedFalse(literals_[i]))
		{
			counter_false_literals++;
		}
		else if (state.assignments_.IsAssignedTrue(literals_[i]))
		{
			counter_true_literals++;
		}
		else
		{
			assert(state.assignments_.IsAssigned(literals_[i]) == false);
			counter_unassigned_literals++;
		}
	}
	return counter_unassigned_literals == 1 && (counter_false_literals + 1 == literals_.Size());
}

bool TwoWatchedClause::HaxPrint(const SolverState &state) const
{
	std::cout << "Printing clause\n";
	for (BooleanLiteral literal : literals_)
	{
		if (literal.IsNegative())
		{
			std::cout << "-";
		}
		std::cout << literal.Variable().index_ << ": ";

		if (state.assignments_.IsAssignedTrue(literal))
		{
			std::cout << "T (" << state.assignments_.GetAssignmentLevel(literal.Variable()) << " " << (state.assignments_.GetAssignmentPropagator(literal.Variable()) != NULL)  << "\n";
		}
		else if (state.assignments_.IsAssignedFalse(literal))
		{
			std::cout << "F (" << state.assignments_.GetAssignmentLevel(literal.Variable()) << " " << (state.assignments_.GetAssignmentPropagator(literal.Variable()) != NULL)  << "\n";
		}
		else
		{
			std::cout << "U\n";
		}
	}
	return false;
}

bool TwoWatchedClause::WatcherComparison(const BooleanLiteral p1, const BooleanLiteral p2, const SolverState &state)
{
	const Assignments &assignments = state.assignments_;

	if (assignments.IsAssignedFalse(p1) && assignments.IsAssignedFalse(p2))
	{
		return assignments.GetAssignmentLevel(p1.Variable()) > assignments.GetAssignmentLevel(p2.Variable());
	}
	else if (assignments.IsAssignedTrue(p1) && assignments.IsAssignedTrue(p2))
	{
		return assignments.GetAssignmentLevel(p1.Variable()) < assignments.GetAssignmentLevel(p2.Variable());
	}
	else if (assignments.IsAssignedTrue(p1) && assignments.IsAssignedFalse(p2))
	{
		return true;
	}
	else if (assignments.IsAssignedFalse(p1) && assignments.IsAssignedTrue(p2))
	{
		return false;
	}
	//at this point at least one of the two literals must be unassigned
	else if (assignments.IsAssignedTrue(p1))
	{
		runtime_assert(assignments.IsAssigned(p2) == false);
		return true;
	}
	else if (assignments.IsAssignedTrue(p2))
	{
		runtime_assert(assignments.IsAssigned(p1) == false);
		return false;
	}
	else
	{
		return assignments.IsAssigned(p1) < assignments.IsAssigned(p2);
	}	
}

bool TwoWatchedClause::updateLBD(const SolverState & state)
{
	if (best_literal_blocking_distance_ <= 2) { return false; } //no reason to update below two since anyway clauses with LBD=2 are never deleted

	int old_best = best_literal_blocking_distance_;
	best_literal_blocking_distance_ = std::min(best_literal_blocking_distance_, computeLBD(literals_, state));
	return old_best != best_literal_blocking_distance_;
}

bool TwoWatchedClause::IsSatisfied(const SolverState & state)
{
	for (BooleanLiteral literal : literals_)
	{
		if (state.assignments_.IsAssignedTrue(literal))
		{
			return true;
		}
	}

	return false;
}

int TwoWatchedClause::computeLBD(std::vector<BooleanLiteral>& literals, const SolverState & state)
{
	static std::vector<int> seen_decision_level(state.GetNumberOfVariables()+1, false);
	if (seen_decision_level.size() < state.GetNumberOfVariables() + 1) { seen_decision_level.resize(state.GetNumberOfAssignedVariables() + 1, false); } //+1 since variables are indexed starting from 1
	
	int counter = 0;
	for (BooleanLiteral lit : literals)
	{
		int decision_level = state.assignments_.GetAssignmentLevel(lit.Variable());
		counter += (decision_level != 0 && seen_decision_level[decision_level] == false);
		seen_decision_level[decision_level] = true;
	}
	//clear the seen_decision_level data structure - all of its values should be false for the next function call
	for (BooleanLiteral lit : literals)
	{
		int i = state.assignments_.GetAssignmentLevel(lit.Variable());
		seen_decision_level[i] = false;
	}
	return counter;
}

int TwoWatchedClause::computeLBD(LiteralVector & literals, const SolverState & state)
{
	static std::vector<int> seen_decision_level(state.GetNumberOfVariables() + 1, false);
	if (seen_decision_level.size() < state.GetNumberOfVariables() + 1) { seen_decision_level.resize(state.GetNumberOfAssignedVariables() + 1, false); } //+1 since variables are indexed starting from 1
																																						//count the number of unique decision levels by using the seen_decision_level to store whether or not a particular decision level has already been counted
	int counter = 0;
	for (BooleanLiteral lit : literals)
	{
		if (state.assignments_.IsAssigned(lit))
		{
			int decision_level = state.assignments_.GetAssignmentLevel(lit.Variable());
			counter += (decision_level != 0 && seen_decision_level[decision_level] == false);
			seen_decision_level[decision_level] = true;
		}
		else //for unassigned literals, pesimistically assume that that each unassigned literal will be set at a different decision level
		{
			counter++;
		}
	}
	//clear the seen_decision_level data structure - all of its values should be false for the next function call
	for (BooleanLiteral lit : literals)
	{
		if (state.assignments_.IsAssigned(lit))
		{
			int decision_level = state.assignments_.GetAssignmentLevel(lit.Variable());
			seen_decision_level[decision_level] = false;
		}		
	}
	return counter;
}

WatchedLiterals TwoWatchedClause::GetWatchedLiterals() const
{
	return WatchedLiterals(literals_[0], literals_[1]);
}

BooleanLiteral TwoWatchedClause::GetOtherWatchedLiteral(BooleanLiteral watched_literal) const
{
	assert(literals_[0] == watched_literal || literals_[1] == watched_literal);
	if (literals_[0] == watched_literal)
	{
		return literals_[1];
	}
	else
	{
		assert(literals_[1] == watched_literal);
		return literals_[0];
	}	
}

int TwoWatchedClause::Size() const
{
	return int(literals_.Size());
}

bool TwoWatchedClause::IsBinary() const
{
	return literals_.Size() == 2;
}

} //end Pumpkin namespace