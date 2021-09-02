#include "propagator_pseudo_boolean_constraints.h"
#include "watch_list_pseudo_boolean.h"
#include "reason_pseudo_boolean_constraint.h"
#include "../../Engine/solver_state.h"

namespace Pumpkin
{

PropagatorPseudoBooleanConstraints::PropagatorPseudoBooleanConstraints(int64_t num_variables):
	PropagatorGeneric(),
	constraint_database_(num_variables),
	failure_constraint_(0)
{
}

ExplanationPseudoBooleanConstraint * PropagatorPseudoBooleanConstraints::ExplainLiteralPropagation(BooleanLiteral propagated_literal, SolverState & state)
{
	assert(state.assignments_.GetAssignmentPropagator(propagated_literal.Variable()) == this);
	uint64_t code = state.assignments_.GetAssignmentCode(propagated_literal.Variable());
	WatchedPseudoBooleanConstraint *propagating_constraint = reinterpret_cast<WatchedPseudoBooleanConstraint*>(code);
	return propagating_constraint->ExplainLiteralPropagation(propagated_literal, state);
}

ExplanationPseudoBooleanConstraint * PropagatorPseudoBooleanConstraints::ExplainFailure(SolverState & state)
{
	assert(failure_constraint_->IsFailing(state));
	//note that the last clause that was detected to be unsatisfied during propagation is stored in failure_clause
	return failure_constraint_->ExplainFailure(state);
}

ReasonGeneric * PropagatorPseudoBooleanConstraints::ReasonLiteralPropagation(BooleanLiteral propagated_literal, SolverState & state)
{
	//todo should update the LBD
	assert(state.assignments_.GetAssignmentPropagator(propagated_literal.Variable()) == this);
	uint64_t code = state.assignments_.GetAssignmentCode(propagated_literal.Variable());
	WatchedPseudoBooleanConstraint *propagating_constraint = reinterpret_cast<WatchedPseudoBooleanConstraint*>(code);
	return new ReasonPseudoBooleanConstraint(propagating_constraint, propagated_literal, state);
}

ReasonGeneric * PropagatorPseudoBooleanConstraints::ReasonFailure(SolverState & state)
{
	//todo should update the LBD
	return new ReasonPseudoBooleanConstraint(failure_constraint_, state);
}

bool PropagatorPseudoBooleanConstraints::PropagateConstraintAtRoot(WatchedPseudoBooleanConstraint * constraint, SolverState & state)
{
	assert(state.GetCurrentDecisionLevel() == 0);
	uint32_t lhs = 0;
	for (size_t i = 0; i < constraint->terms_.size(); i++) 
	{ 
		lhs += constraint->terms_[i].coefficient*(state.assignments_.IsAssignedFalse(constraint->terms_[i].literal)==false); //even at root some literals can be assigned
	}

	//the constraint is violated beyond any hope, rip
	if (lhs < constraint->right_hand_side_) 
	{ 
		return false; 
	}
	else
	{
		assert(constraint->right_hand_side_ <= lhs); //to be sure overflow will not happen in the next line
		uint32_t slack = lhs - constraint->right_hand_side_;
		//propagate every unassigned literal with a coefficients greater than the slack
		for (size_t i = 0; i < constraint->terms_.size(); i++)
		{
			if (slack < constraint->terms_[i].coefficient) //assigning this literal to false would break the constraint, hence propagate to true
			{
				if (state.assignments_.IsAssigned(constraint->terms_[i].literal)) { continue; } //ignore already assigned ones

				uint64_t code = reinterpret_cast<uint64_t>(constraint);
				bool success = state.AddUnitClause(constraint->terms_[i].literal); //todo consider literals that were propagated together?
				if (success == false) { return false; }
			}
		}
		return true;
	}
}

bool PropagatorPseudoBooleanConstraints::PropagateLiteral(BooleanLiteral true_literal, SolverState & state)
{
	return PropagateLiteral2(true_literal, state);
	
	assert(state.assignments_.IsAssignedTrue(true_literal));


	//effectively remove all watches from this true_literal
	//then go through the previous watches one by one and insert them as indicated (some might be placed back in the watch list of this true_literal)
	//if a conflict takes place, put back the remaining clauses into the watch list of this true_literal and report the conflict

	WatchListPseudoBoolean &watch_list = constraint_database_.watch_list_;
	std::vector<WatcherPseudoBoolean> &watches = watch_list[~true_literal];

	size_t end_position = 0;  //effectively, we are resizing the watch list to zero for this literal, and in the loop we will add some of the old watches
	for (size_t current_index = 0; current_index < watches.size(); current_index++)
	{
		WatchedPseudoBooleanConstraint * watched_constraint = watches[current_index].constraint_;
		std::vector<Term> &terms = watched_constraint->terms_;
		uint32_t right_hand_side = watched_constraint->right_hand_side_;

		uint32_t max_watched_coefficient = 0;
		uint32_t sum_of_true_watched_literals = 0;
		uint32_t sum_of_watched_literals = 0;
		Term * term_with_literal = NULL;

		uint32_t dbg_false_watches = 0;
		
		/*uint32_t sum_nonfalse_nonwatches = 0;

		for (size_t i = watched_constraint->num_watches_; i < watched_constraint->terms_.size_(); i++)
		{
			sum_nonfalse_nonwatches += (terms[i].coefficient * (state.assignments_.IsAssignedFalse(terms[i].literal)==false));
		}*/

		//compute the left hand side and other helper data
		//in fact, it does not compute everything properly if it detects that the constraint is satisfiable already
		for (Term* iter = watched_constraint->BeginWatchedTerms(); iter != watched_constraint->EndWatchedTerms(); iter++)
		{
			//debug var
			dbg_false_watches += (iter->coefficient * state.assignments_.IsAssignedFalse(iter->literal));


			//only true literals contribute to the left hand side satisfaction
			sum_of_true_watched_literals += (iter->coefficient * state.assignments_.IsAssignedTrue(iter->literal));
			if (sum_of_true_watched_literals >= right_hand_side) { break; }

			sum_of_watched_literals += iter->coefficient;
			
			if (iter->literal == ~true_literal) { term_with_literal = iter; }
			else { max_watched_coefficient = std::max(iter->coefficient, max_watched_coefficient); } //the input literal will be removed from the watches possibly, so no need to consider it
		}

		//constraint is satisfied already, no propagation can take place
		if (sum_of_true_watched_literals >= right_hand_side)
		{
			watches[end_position] = watches[current_index]; //keep the watch
			end_position++;
			//should I check if I can remove this watcher? todo I think it is not necessary, the number of watchers will shrink eventually if necessary with the next loop
			continue; //go to the next constraint
		}

		assert(term_with_literal != NULL); //sanity check, the literal must be present

		//remove the input literal from the watch since it is false in the constraint
		//adjust the sum, swap with the last watched literals, and decrement the counter
		sum_of_watched_literals -= term_with_literal->coefficient; //use term_with_literal before swapping
		assert(state.assignments_.IsAssignedFalse(term_with_literal->literal));
		dbg_false_watches -= term_with_literal->coefficient;
		std::swap(*term_with_literal, terms[watched_constraint->num_watches_ - 1]);
		watched_constraint->num_watches_--;
		//todo: add looping?

		//the watchers aim to satisfy the condition: \sum watched_literals[i] - max_watched_coefficient >= right_hand_side
		//if that is met, then no propagation can occur, otherwise something must propagate
		//we set the watchers in order, todo perhaps it could be worth considering if there is a better scheme?
		//note that this might shrink the watch list, it might not be necessary to add a new watcher

		//Maybe we could remove some watches......
		
		int i = watched_constraint->num_watches_ + 1; //skip the just removed literal
		while (i < terms.size() && sum_of_watched_literals - max_watched_coefficient - dbg_false_watches < right_hand_side)
		{
			//can the literal be used as a watcher? Note these is no point in adding already falsified literals to the watch
			if (state.assignments_.IsAssignedFalse(terms[i].literal) == false) //not a falsified literal
			{
				//add as a watcher and update helper data
				watch_list.Add(terms[i].literal, watched_constraint);
				max_watched_coefficient = std::max(terms[i].coefficient, max_watched_coefficient);
				sum_of_true_watched_literals += (terms[i].coefficient * state.assignments_.IsAssignedTrue(terms[i].literal)); //might contribute to the left hand side now
				sum_of_watched_literals += terms[i].coefficient;
				//properly set it as a watcher in the contraint
				std::swap(terms[i], terms[watched_constraint->num_watches_]);
				watched_constraint->num_watches_++;
				//TODO: it could be that the constraint is unsat due to other enqueues ->  does it make sense to make the test now, or to leave it for another propagation that will follow to discover this? I suppose it is not worth it since conflicts are rare in the search
			}
			i++;
		}

		//the last falsified literal to leave the constraint should be responsible for restoring the correct state -> collect which constraints need tobe investigated first, and make only one call per constraint

		//watch list property is successfully restored - no propagation is possible
		//note that since we did not increment end_position, we effectively removed this constraint from the watch of the input true_literal
		if (sum_of_watched_literals - max_watched_coefficient - dbg_false_watches >= right_hand_side)
		{
			continue;
		}

		//the watch list property must be restored
		//make note of the number of current watches
		//at this point, every literal not in the current watches has been set to false, so we can avoid inspecting them in the code below
		size_t start_of_falsified_literals = watched_constraint->num_watches_;
		
		//there is no value for the current propagation procedure to add watches to false literals
		//adding them now does not have an effect on correctness, but the solver might wasting its time looking into this constraint again
		//this refers to literals that were enqueued to false on the current decision level
		//these need to be added once all propagation is done for this decision level

		//TODO I did not handle these yet
		//how to add it: add time stamps to watchers? and record time step at start? so when you read a new literal that is at this decision level, skip watches that were added afterwards?
		//or keep these in a separate vector and once propagation is done, readd everything 

		//wait...there is a danger that I can add the same constraint for the same variable multiple times like this?
		//...I think no, because once you add something to the watches, it stays watched
		//...however, I do remove watches for false literals...uh!
		//...but then it's not possible to add the constraint in the watch list twice.
		//remember you cannot process a literal twice at the same level, so propagation must terminate

		//also remember you only remove the current literal, so if the watched property is not met, you will readd the literal below
		
		//but the only time watched literal property cannot be met is if it is not possible to fully restore the watched literals property
		//in that case, propagation will make the constraint satisfiable?

		//it's good to remove watches since upon backtracking, I will have less watchers...meaning propagation is guaranteed to be faster...?
				
		i = watched_constraint->num_watches_;
		while (i < terms.size() && sum_of_watched_literals - max_watched_coefficient - dbg_false_watches < right_hand_side)
		{
			assert(state.assignments_.IsAssignedFalse(terms[i].literal));
			if (terms[i].literal != ~true_literal)
			{
				watch_list.Add(terms[i].literal, watched_constraint);
			}
			else
			{
				watches[end_position] = watches[current_index];
				end_position++;
			}
			dbg_false_watches += terms[i].coefficient;
			sum_of_watched_literals += terms[i].coefficient;
			watched_constraint->num_watches_++;
			i++;
		}

		assert(terms.size() == watched_constraint->num_watches_ || sum_of_watched_literals - max_watched_coefficient - dbg_false_watches >= right_hand_side); //must be possible - worst case all literals are watchers
				
		//if we detected the constraint is satisfied, it cannot propagate, go to the next constraint
		//note that since we did not increment end_position, we effectively removed this constraint from the watch of the input true_literal
		//note that this must be done after restoring the watched property, if possible, not before!
		//todo not sure about this part -> no need to restore the property if it is sat, so the previous loop can be removed -> remember, if one of the true watches become false, then we will trigger restoring the watches...
		if (sum_of_true_watched_literals >= right_hand_side)
		{ 
			assert(watched_constraint->num_watches_ >= 2); //only trivial constraints that are satisfeid can have less than two watches, and these should not be used
			continue; 
		}
		
		//at this point, every literal that is not a watcher is assigned false
		//recall this is the case because the watcher property could not be restored using nonfalsified literals
		//thus propagation among the watched literals must take place to ensure the constraint can be satisfied in the future
		//note that some watchers might be false

		//prepare for propagation

		//todo this computation could have been done on the fly? Probably not a bottleneck anyway
		uint32_t sum_of_nonfalsified_literals = 0;		
		for (i = 0; i < start_of_falsified_literals; i++)
		{
			sum_of_nonfalsified_literals += (terms[i].coefficient*(state.assignments_.IsAssignedFalse(terms[i].literal) == false));
		}

		//the constraint is violated beyond any hope, rip
		if (sum_of_nonfalsified_literals < right_hand_side)
		{
			//restore the remaining watchers
			for (size_t k = current_index + 1; k < watches.size(); k++)
			{
				watches[end_position] = watches[k];
				end_position++;
			}
			watches.resize(end_position);
			failure_constraint_ = watched_constraint;
			return false;
		}
		else
		{
			assert(right_hand_side <= sum_of_nonfalsified_literals); //to be sure overflow will not happen in the next line
			uint32_t slack = sum_of_nonfalsified_literals - right_hand_side;
			//propagate every unassigned literal with a coefficients greater than the slack
			for(i = 0; i < start_of_falsified_literals; i++)
			{
				if (state.assignments_.IsAssigned(terms[i].literal)) { continue; } //only looking for unassigned ones
				
				if (slack < terms[i].coefficient) //assigning this literal to false would break the constraint, hence propagate to true
				{
					uint64_t code = reinterpret_cast<uint64_t>(watched_constraint);
					state.EnqueuePropagatedLiteral(terms[i].literal, this, code); //todo consider literals that were propagated together?
				}
			}
		}

		assert(!watched_constraint->IsUnsat(state));
		//assert(watched_constraint->IsCorrect(state));	

		//assert(hax_constraint_ == NULL || hax_constraint_->IsCorrect(state));
	}
	watches.resize(end_position);

	for (size_t current_index = 0; current_index < watches.size(); current_index++)
	{
	//	assert(watches[current_index].constraint_->IsCorrect(state));
	}

	return true; //no conflicts detected	
}


bool PropagatorPseudoBooleanConstraints::PropagateLiteral2(BooleanLiteral true_literal, SolverState & state)
{
	assert(state.assignments_.IsAssignedTrue(true_literal));
	
	//effectively remove all watches from this true_literal
	//then go through the previous watches one by one and insert them as indicated (some might be placed back in the watch list of this true_literal)
	//if a conflict takes place, put back the remaining clauses into the watch list of this true_literal and report the conflict

	WatchListPseudoBoolean &watch_list = constraint_database_.watch_list_;
	std::vector<WatcherPseudoBoolean> &watches = watch_list[~true_literal];

	size_t end_position = 0;  //effectively, we are resizing the watch list to zero for this literal, and in the loop we will add some of the old watches
	for (size_t current_index = 0; current_index < watches.size(); current_index++)
	{
		WatchedPseudoBooleanConstraint * watched_constraint = watches[current_index].constraint_;
		std::vector<Term> &terms = watched_constraint->terms_;
		uint32_t right_hand_side = watched_constraint->right_hand_side_;

		//compute all the helper data
		//in fact, it does not compute everything properly if it detects that the constraint is satisfiable already
		uint32_t max_watched_coefficient = 0;
		uint32_t sum_of_true_watched_literals = 0;
		uint32_t sum_of_watched_literals = 0;
		Term * term_with_literal = NULL;
		bool forced_false_watcher_exists = false;

		for (Term* iter = watched_constraint->BeginWatchedTerms(); iter != watched_constraint->EndWatchedTerms(); iter++)
		{
			forced_false_watcher_exists |= (state.assignments_.IsAssignedFalse(iter->literal)
				&& state.assignments_.GetTrailPosition(iter->literal.Variable()) < state.assignments_.GetTrailPosition(true_literal.Variable()));
			
			sum_of_true_watched_literals += (iter->coefficient * state.assignments_.IsAssignedTrue(iter->literal));

			if (sum_of_true_watched_literals >= right_hand_side) { break; }

			sum_of_watched_literals += iter->coefficient;

			if (iter->literal == ~true_literal) { term_with_literal = iter; }
			else { max_watched_coefficient = std::max(iter->coefficient, max_watched_coefficient); } //the input literal will be removed from the watches possibly, so no need to consider it
		}

		//constraint is satisfied already, no propagation can take place
		if (sum_of_true_watched_literals >= right_hand_side)
		{
			watches[end_position] = watches[current_index]; //keep the watch
			end_position++;
			//assert(hax_constraint_ == NULL || hax_constraint_->IsCorrect(state));
			//should I check if I can remove this watcher? todo I think it is not necessary, the number of watchers will shrink eventually if necessary with the next loop
			continue; //go to the next constraint
		}

		assert(term_with_literal != NULL); //sanity check, the literal must be present


		if (forced_false_watcher_exists)
		{
			for (size_t i = watched_constraint->num_watches_; i < terms.size(); i++)
			{
				if (state.assignments_.IsAssignedFalse(terms[i].literal)==false)
				{
					watched_constraint->Print(state);
				}
				assert(state.assignments_.IsAssignedFalse(terms[i].literal));
			}
		}

		if (!forced_false_watcher_exists)
		{
			//remove the input literal from the watch since it is false in the constraint
			//adjust the sum, swap with the last watched literals, and decrement the counter
			sum_of_watched_literals -= term_with_literal->coefficient; //use term_with_literal before swapping
			assert(state.assignments_.IsAssignedFalse(term_with_literal->literal));
			std::swap(*term_with_literal, terms[watched_constraint->num_watches_ - 1]);
			watched_constraint->num_watches_--;
			//todo: add looping?

			//the watchers aim to satisfy the condition: \sum watched_literals[i] - max_watched_coefficient >= right_hand_side
			//if that is met, then no propagation can occur, otherwise something must propagate
			//we set the watchers in order, todo perhaps it could be worth considering if there is a better scheme?
			//note that this might shrink the watch list, it might not be necessary to add a new watcher
			
			//here we assume that no watcher is false -> this may not be true, but at this point we do not have any false watchers that cannot be fixed
			//and the....explain why it is fine

			//try to restore the watched property using only nonfalsified literals
			int i = watched_constraint->num_watches_ + 1; //skip the just removed literal
			while (i < terms.size() && sum_of_watched_literals - max_watched_coefficient < right_hand_side)
			{
				//can the literal be used as a watcher? Note there is no point in adding already falsified literals to the watch
				if (state.assignments_.IsAssignedFalse(terms[i].literal) == false) //not a falsified literal
				{
					//add as a watcher and update helper data
					watch_list.Add(terms[i].literal, watched_constraint);
					max_watched_coefficient = std::max(terms[i].coefficient, max_watched_coefficient);
					sum_of_true_watched_literals += (terms[i].coefficient * state.assignments_.IsAssignedTrue(terms[i].literal)); //might contribute to the left hand side now
					sum_of_watched_literals += terms[i].coefficient;
					//properly set it as a watcher in the contraint
					std::swap(terms[i], terms[watched_constraint->num_watches_]);
					watched_constraint->num_watches_++;
					//TODO: it could be that the constraint is unsat due to other enqueues ->  does it make sense to make the test now, or to leave it for another propagation that will follow to discover this? I suppose it is not worth it since conflicts are rare in the search
				}
				i++;
			}

			//the last falsified literal to leave the constraint should be responsible for restoring the correct state -> collect which constraints need tobe investigated first, and make only one call per constraint

			//watch list property is successfully restored - no propagation is possible
			//note that since we did not increment end_position, we effectively removed this constraint from the watch of the input true_literal
			if (sum_of_watched_literals - max_watched_coefficient >= right_hand_side)
			{
				continue;
			}

			//restore as we can

			//the watch list property must be restored
			//make note of the number of current watches
			//at this point, every literal not in the current watches has been set to false, so we can avoid inspecting them in the code below
			size_t start_of_falsified_literals = watched_constraint->num_watches_;

			//there is no value for the current propagation procedure to add watches to false literals
			//adding them now does not have an effect on correctness, but the solver might wasting its time looking into this constraint again
			//this refers to literals that were enqueued to false on the current decision level
			//these need to be added once all propagation is done for this decision level

			//TODO I did not handle these yet
			//how to add it: add time stamps to watchers? and record time step at start? so when you read a new literal that is at this decision level, skip watches that were added afterwards?
			//or keep these in a separate vector and once propagation is done, readd everything 

			//wait...there is a danger that I can add the same constraint for the same variable multiple times like this?
			//...I think no, because once you add something to the watches, it stays watched
			//...however, I do remove watches for false literals...uh!
			//...but then it's not possible to add the constraint in the watch list twice.
			//remember you cannot process a literal twice at the same level, so propagation must terminate

			//also remember you only remove the current literal, so if the watched property is not met, you will readd the literal below

			//but the only time watched literal property cannot be met is if it is not possible to fully restore the watched literals property
			//in that case, propagation will make the constraint satisfiable?

			//it's good to remove watches since upon backtracking, I will have less watchers...meaning propagation is guaranteed to be faster...?


			//I think if you could not restore the property, put this literal back into the watchers
			i = watched_constraint->num_watches_;
			while (i < terms.size())
			{
				assert(state.assignments_.IsAssignedFalse(terms[i].literal));
				if (terms[i].literal == ~true_literal)
				{					
					//we are readding the input literal to the watch list
					watches[end_position] = watches[current_index];
					end_position++;
					sum_of_watched_literals += terms[i].coefficient;
					std::swap(terms[i], terms[watched_constraint->num_watches_]);
					watched_constraint->num_watches_++;
					break;
				}
				i++;
			}
			assert(terms[watched_constraint->num_watches_-1].literal == ~true_literal);

			/*i = watched_constraint->num_watches_;
			while (i < terms.size() && sum_of_watched_literals - max_watched_coefficient < right_hand_side)
			{
				assert(state.assignments_.IsAssignedFalse(terms[i].literal));
				if (terms[i].literal != ~true_literal)
				{
					watch_list.Add(terms[i].literal, watched_constraint);
				}
				else //we are readding the input literal to the watch list
				{
					watches[end_position] = watches[current_index];
					end_position++;
				}
				sum_of_watched_literals += terms[i].coefficient;
				watched_constraint->num_watches_++;
				i++;
			}*/
			//assert(sum_of_watched_literals - max_watched_coefficient >= right_hand_side);

			//if we detected the constraint is satisfied, it cannot propagate, go to the next constraint
			//note that since we did not increment end_position, we effectively removed this constraint from the watch of the input true_literal
			//note that this must be done after restoring the watched property, if possible, not before!
			//todo not sure about this part -> no need to restore the property if it is sat, so the previous loop can be removed -> remember, if one of the true watches become false, then we will trigger restoring the watches...
			if (sum_of_true_watched_literals >= right_hand_side)
			{
				assert(watched_constraint->num_watches_ >= 2); //only trivial constraints that are satisfeid can have less than two watches, and these should not be used
				continue;
			}
		}
		else
		{
			//keep the watch, do not remove it
			watches[end_position] = watches[current_index];
			end_position++;
		}


		//at this point, every literal that is not a watcher is assigned false
		//recall this is the case because the watcher property could not be restored using nonfalsified literals
		//thus propagation among the watched literals must take place to ensure the constraint can be satisfied in the future
		//note that some watchers might be false

		//prepare for propagation

		//todo this computation could have been done on the fly? Probably not a bottleneck anyway
		uint32_t sum_of_nonfalsified_literals = 0; //only need to look at watches perhaps?
		//for (size_t i = 0; i < terms.size(); i++) //could limit this using start_of_fals todo
		for (size_t i = 0; i < watched_constraint->num_watches_; i++)
		{
			sum_of_nonfalsified_literals += (terms[i].coefficient*(state.assignments_.IsAssignedFalse(terms[i].literal) == false));
		}

		//the constraint is violated beyond any hope, rip
		if (sum_of_nonfalsified_literals < right_hand_side)
		{
			//restore the remaining watchers
			for (size_t k = current_index + 1; k < watches.size(); k++)
			{
				watches[end_position] = watches[k];
				end_position++;
			}
			watches.resize(end_position);
			failure_constraint_ = watched_constraint;
			return false;
		}
		else
		{
			assert(right_hand_side <= sum_of_nonfalsified_literals); //to be sure overflow will not happen in the next line
			uint32_t slack = sum_of_nonfalsified_literals - right_hand_side;
			//propagate every unassigned literal with a coefficients greater than the slack
			//for (size_t i = 0; i < terms.size(); i++) //could limit this todo using start_of_falsified_literals
			for (size_t i = 0; i < watched_constraint->num_watches_; i++)
			{
				if (state.assignments_.IsAssigned(terms[i].literal)) { continue; } //only looking for unassigned ones

				if (slack < terms[i].coefficient) //assigning this literal to false would break the constraint, hence propagate to true
				{
					uint64_t code = reinterpret_cast<uint64_t>(watched_constraint);
					state.EnqueuePropagatedLiteral(terms[i].literal, this, code); //todo consider literals that were propagated together?
				}
			}
		}

		assert(!watched_constraint->IsUnsat(state));
		//assert(watched_constraint->IsCorrect(state));	

		//assert(hax_constraint_ == NULL || hax_constraint_->IsCorrect(state));
	}
	watches.resize(end_position);

	for (size_t current_index = 0; current_index < watches.size(); current_index++)
	{
		//	assert(watches[current_index].constraint_->IsCorrect(state));
	}

	return true; //no conflicts detected	
}
void PropagatorPseudoBooleanConstraints::GrowDatabase() {
  constraint_database_.watch_list_.Grow();
}

} //end Pumpkin namespace