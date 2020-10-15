#include "propagator_clausal.h"
#include "explanation_clausal.h"
#include "reason_clausal.h"
#include "../../Engine/solver_state.h"

namespace Pumpkin
{

PropagatorClausal::PropagatorClausal(int64_t num_variables, double decay_factor):
	PropagatorGeneric(),
	clause_database_(num_variables, decay_factor),
	failure_clause_(0)
{
}

/*TwoWatchedClause * PropagatorClausal::Propagate(SolverState & state)
{
	while (IsPropagationComplete(state) == false)
	{
		BooleanLiteral propagation_literal = GetAndPopNextLiteralToPropagate(state);
		TwoWatchedClause *conflict_clause = PropagateLiteral(propagation_literal, state);
		if (conflict_clause != NULL)
		{
			return conflict_clause;
		}
	}
	return NULL; //no conflicts occurred during propagation
}*/
/*
void PropagatorClausal::Synchronise(SolverState & state)
{
	//actually, if I keep this propagator first, I do not need to do the min...TODO
	next_position_on_trail_to_propagate_ = std::min(next_position_on_trail_to_propagate_, state.GetNumberOfAssignedVariables());
}*/

ExplanationGeneric * PropagatorClausal::ExplainLiteralPropagation(BooleanLiteral literal, SolverState & state)
{
	assert(state.assignments_.GetAssignmentPropagator(literal.Variable()) == this);
	uint64_t code = state.assignments_.GetAssignmentCode(literal.Variable());	
	TwoWatchedClause *propagating_clause = reinterpret_cast<TwoWatchedClause*>(code);
	UpdateClauseActivityAndLBD(propagating_clause, state);
	return propagating_clause->ExplainLiteralPropagation(literal);
}

ExplanationGeneric * PropagatorClausal::ExplainFailure(SolverState &state)
{
	//note that the last clause that was detected to be unsatisfied during propagation is stored in failure_clause
	UpdateClauseActivityAndLBD(failure_clause_, state);
	return failure_clause_->ExplainFailure();
}

ReasonGeneric * PropagatorClausal::ReasonLiteralPropagation(BooleanLiteral literal, SolverState & state)
{
	assert(state.assignments_.GetAssignmentPropagator(literal.Variable()) == this);
	uint64_t code = state.assignments_.GetAssignmentCode(literal.Variable());
	TwoWatchedClause *propagating_clause = reinterpret_cast<TwoWatchedClause*>(code);
	UpdateClauseActivityAndLBD(propagating_clause, state);
	return new ReasonClausal(propagating_clause);
	//return new ReasonClausal(propagating_clause, literal);
}

ReasonGeneric * PropagatorClausal::ReasonFailure(SolverState & state)
{
	return new ReasonClausal(failure_clause_);
}

bool PropagatorClausal::PropagateLiteral(BooleanLiteral true_literal, SolverState &state)
{
	assert(state.assignments_.IsAssignedTrue(true_literal));

	//effectively remove all watches from this true_literal
	//then go through the previous watches one by one and insert them as indicated (some might be placed back in the watch list of this true_literal)
	//if a conflict takes place, put back the remaining clauses into the watch list of this true_literal and report the conflict

	WatchList &watch_list = clause_database_.watch_list_;
	std::vector<WatcherClause> &watches = watch_list[~true_literal];

	size_t end_position = 0;  //effectively, we are resizing the watch list to zero for this literal, and in the loop we will add some of the old watches
	for (size_t current_index = 0; current_index < watches.size(); current_index++)
	{
		assert(watches[current_index].clause_->literals_[0] == ~true_literal || watches[current_index].clause_->literals_[1] == ~true_literal); //one of the watched literals must be the opposite of the input true literal

		//inspect if the cached true_literal is already set to true
		//if so, no need to go further in the memory to check the clause
		//often this literal will be true in practice so it is a good heuristic to check
		if (state.assignments_.IsAssignedTrue(watches[current_index].cached_literal_))
		{
			watches[end_position] = watches[current_index]; //keep the watch
			end_position++;
			continue;
		}

		TwoWatchedClause * watched_clause = watches[current_index].clause_;
		
		//clause propagation starts here

		//place the considered literal at position 1 for simplicity
		if (watched_clause->literals_[0] == ~true_literal) { std::swap(watched_clause->literals_[0], watched_clause->literals_[1]); }

		//check the other watched literal to see if the clause is already satisfied
		if (state.assignments_.IsAssignedTrue(watched_clause->literals_[0]) == true) 
		{
			watches[current_index].cached_literal_ = watched_clause->literals_[0]; //take the true literal as the new cache
			watches[end_position] = watches[current_index]; //keep the watch
			end_position++;
			continue; //the clause is satisfied, no propagation can take place, go to the next clause
		}

		//look for another nonfalsified literal to replace one of the watched literals
		bool found_new_watch = false;
		for (int i = 2; i < watched_clause->literals_.Size(); i++) //current_index = 2 since we are skipping the two watched literals
		{
			if (state.assignments_.IsAssignedFalse(watched_clause->literals_[i]) == false) //not assigned false//not assigned false, can be either true or unassigned
			{
				//TODO: would it make sense to set the cached literal here if this new literal will be set to true?
				std::swap(watched_clause->literals_[1], watched_clause->literals_[i]); //replace the watched literal
				watch_list.AddClauseToWatch(watched_clause->literals_[1], watched_clause); //add the clause to the watch of the new watched literal
				found_new_watch = true;
				break; //no propagation is taking place, go to the next clause. Note that since we did not increment end_position, we effectively removed this clause from the watch of the input true_literal
				//TODO: it could be that literal[0] is false, and that the current clause is actually unit with the new watched literal ->  does it make sense to make the test now, or to leave it for another propagation that will follow to discover this? I suppose it is not worth it since conflicts are rare in the search
			}
		}

		if (found_new_watch) { continue; }

		//at this point, nonwatched literals and literal[1] are assigned false
		//therefore, this class is unit if literal_[0] is not assigned, or satisfied/falsified if literal_[0] is true/false
		//the latter happens if literal[0] was enqueueud at some point but has not yet been propagated
		
		//keep the current watch for this literal
		watches[end_position] = watches[current_index]; 
		end_position++;	

		//try to enqueue the propagated assignment
		uint64_t code = reinterpret_cast<uint64_t>(watched_clause); //the code will simply be a pointer to the propagating clause
		bool enqueue_success = state.EnqueuePropagatedLiteral(watched_clause->literals_[0], this, code);
		//it was not possible to even enqueue, conflict occures
		if (enqueue_success == false) 
		{
			//restore the remaining watchers
			for (size_t k = current_index + 1; k < watches.size(); k++)
			{
				watches[end_position] = watches[k];
				end_position++;
			}
			watches.resize(end_position);
			failure_clause_ = watched_clause;
			return false;
		}
	}	
	watches.resize(end_position);
	return true; //no conflicts detected				 
}

void PropagatorClausal::UpdateClauseActivityAndLBD(TwoWatchedClause * clause, SolverState &state)
{
	if (clause->is_learned_)
	{
		state.propagator_clausal_.clause_database_.BumpClauseActivity(clause);
		bool updated = clause->updateLBD(state);
		if (updated == true && clause->best_literal_blocking_distance_ <= 30)
		{
			clause->lbd_update_protection_ = true;
		}
	}
}

/*
BooleanLiteral PropagatorClausal::GetAndPopNextLiteralToPropagate(SolverState & state)
{
	BooleanLiteral return_literal = state.GetLiteralFromTrailAtPosition(next_position_on_trail_to_propagate_);
	next_position_on_trail_to_propagate_++;
	return return_literal;
}

bool PropagatorClausal::IsPropagationComplete(SolverState & state)
{
	assert(next_position_on_trail_to_propagate_ <= state.GetNumberOfAssignedVariables());
	return next_position_on_trail_to_propagate_ == state.GetNumberOfAssignedVariables();
}*/

} //end Pumpkin namespace