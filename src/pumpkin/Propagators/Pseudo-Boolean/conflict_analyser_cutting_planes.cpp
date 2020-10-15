#include "conflict_analyser_cutting_planes.h"
#include "../../Basic Data Structures/learned_pseudo_boolean_constraint_iterator.h"
#include "../../Basic Data Structures/runtime_assert.h"
#include "../../Engine/constraint_satisfaction_solver.h"

#include <fstream>
#include <numeric>

namespace Pumpkin
{

ConflictAnalyserCuttingPlanes::ConflictAnalyserCuttingPlanes(size_t num_variables):
	learned_pseudo_boolean_(num_variables), seen_(num_variables, false)
{
}

ConflictAnalysisResultPseudoBoolean ConflictAnalyserCuttingPlanes::AnalyseConflict(PropagatorGeneric * conflict_propagator, SolverState &state)
{
	runtime_assert(conflict_propagator != NULL);
	assert(CheckDataStructures()); //ensuring the auxiliary data structures are appropriately set
	
	//not using glucose bumping for now
	//use_glucose_bumping_ = false; //temporary hax

	static int gooby = 0;
	gooby++;

	//initialise the analysis with the conflict explanation
	//recall this updates learned_clause_literals_, the num_current_decision_level_literals_, and the backtrack_level_
	InitialiseConflictAnalysis(conflict_propagator, state);
	runtime_assert(num_current_decision_level_literals_ >= 1); //there has to be at least one literal from the current decision level responsible for the failure -> this is not necessary? It could be a root problem?

	//not sure if the previous comment still holds -> but in any case the conjunction view should go away

	//we start inspecting the implication graph of conflict analysis in this loop
	//the loop goes until the current learned constraint propagates something that is in conflict with the current trail
	//todo I think it is guaranteed to get a conflict until it inspects all the decisions
	std::vector<BooleanLiteral> conflicting_literals;
	while (num_current_decision_level_literals_ >= 1) //they propose while no literals from the current or multiple of them -> check the reasoning
	{
		BooleanLiteral resolution_literal = FindNextReasonLiteralOnTheTrail(state); //recall this method will lower the num_current_decision_level_literals counter
		conflict_propagator = state.assignments_.GetAssignmentPropagator(resolution_literal.Variable());
		ProcessConflictPropagator(conflict_propagator, resolution_literal, state);
		conflicting_literals = GetConflictingPropagation(state);
		if (conflicting_literals.size() > 0) { break; } //if the constraint would have propagated something differently, break, and let the search continue from that point. I guess I should not always break, but only break if whatever was propagated was enough to force the clause into a conflicting state?
	}
	runtime_assert(conflicting_literals.size() > 0);
	int backtrack_level = state.assignments_.GetAssignmentLevel(conflicting_literals.back().Variable());
	for (BooleanLiteral lit : conflicting_literals) { state.variable_selector_.BumpActivity(lit.Variable()); } //for now all literals are getting the same bump - todo not sure if I should bump all literals - > I think I already did?
	//todo: not sure if this rounding step is necessary? learned_pseudo_boolean_.RoundToOne(~unique_implication_point_literal, state); //todo, is this correct? Also, should it be the negation? What if it is not present in the conflict?
	assert(learned_pseudo_boolean_.CheckZeroCoefficients()); //not sure about this assert? I think there was a problem with removing zero coefficients since we need to know that we considered the literal at some point? I think we do not need to know that anymore
	//learned_pseudo_boolean_.RemoveZeroCoefficients();

	//if (use_glucose_bumping_) { PerformGlucoseVariableBumping(); }

	ConflictAnalysisResultPseudoBoolean return_value(learned_pseudo_boolean_, backtrack_level); 
	return_value.forced_assignments_at_root_node = GetNewRootPropagations(state);
	return_value.backtrack_level = backtrack_level;
	return_value.propagations_at_backtrack_level = conflicting_literals;

	ClearDataStructures();

	return return_value;
}

void ConflictAnalyserCuttingPlanes::ProcessConflictAnalysisResult(ConflictAnalysisResultPseudoBoolean & result, SolverState &state)
{
	//unit clauses are treated in a special way, somewhat hacky: they are added as decision literals at decision level 0 - this might change in the future if a better idea presents itself
	if (result.forced_assignments_at_root_node.size() > 0)
	{
		//counters_.unit_clauses_learned++; //todo not sure if I should keep this counter here - could easily read from the trail the number of unit literals learned
		if (state.GetCurrentDecisionLevel() != 0) { state.Backtrack(0); }
		for (BooleanLiteral literal : result.forced_assignments_at_root_node) 
		{
			state.EnqueueDecisionLiteral(literal); 
			state.UpdateMovingAveragesForRestarts(1);
		}
	}
	else if (result.learned_pseudo_boolean.IsClause())
	{
		Disjunction learned_literals = result.learned_pseudo_boolean.GetLiterals();
		state.AddLearnedClauseToDatabase(learned_literals);
	}
	else
	{
		runtime_assert(result.propagations_at_backtrack_level.size() > 0);

		int lbd = result.learned_pseudo_boolean.ComputeLBD(state) - 1; //this is not true, it may be that we jump more than one decision level so we'd possibly ignore some of the variable assigned
		state.UpdateMovingAveragesForRestarts(lbd);

		WatchedPseudoBooleanConstraint * learned_constraint = AddLearnedPseudoBooleanConstraintToDatabase(result.learned_pseudo_boolean, state);
		
		state.Backtrack(result.backtrack_level);

		//I think I should ask the propagators to do this kind of setting rather than me hacking it here
		for (BooleanLiteral literal : result.propagations_at_backtrack_level)
		{
			state.EnqueuePropagatedLiteral(literal, &state.propagator_pseudo_boolean_, reinterpret_cast<uint64_t>(learned_constraint)); //todo can this be done cleaner, without having to refer to how 'code' is interpreted by the propagator?
		}

		//need to update the LBD		
		runtime_assert(lbd == result.learned_pseudo_boolean.ComputeLBD(state)); //this will fail
	}
}

void ConflictAnalyserCuttingPlanes::Grow()
{
	learned_pseudo_boolean_.Grow();
	seen_.push_back(false);
}

void ConflictAnalyserCuttingPlanes::InitialiseConflictAnalysis(PropagatorGeneric * conflict_propagator, SolverState & state)
{
	runtime_assert(conflict_propagator != NULL);

	ReasonGeneric *reason_constraint = conflict_propagator->ReasonFailure(state);

	//it is important to add the right hand side immediately since the methods of learned_pseudo_boolean automatically saturate after each operation
	learned_pseudo_boolean_.AddToRightHandSide(reason_constraint->GetRightHandSide());
	for (int i = 0; i < reason_constraint->Size(); i++)
	{
		Term reason_term = (*reason_constraint)[i];
		BooleanVariable reason_variable = reason_term.literal.Variable();

		//should consider removing root level assignments from the problem
		//int lvl = state.assignments_.GetAssignmentLevel(reason_variable);
		//runtime_assert(lvl > 0); //not happy with these asserts here, should be tested in the method that produces these
		//runtime_assert(reason_term.coefficient != 0);

		learned_pseudo_boolean_.AddTerm(reason_term);
	}
	runtime_assert(!learned_pseudo_boolean_.CheckOverflow());
	//runtime_assert(learned_pseudo_boolean_.GetRightHandSide() == 1); //for pure cnf files this holds, but for maxsat, the objective is nonCNF //I think this is just for my current tests? Since I'm giving a CNF, I can only expect these kinds of constraints back

	//this should be done incrementally, for now we just brute force it kek
	num_current_decision_level_literals_ = 0;
	LearnedPseudoBooleanConstraintIterator iterator = learned_pseudo_boolean_.GetIterator();
	while (iterator.IsValid())
	{
		BooleanVariable variable = iterator.Variable();
		num_current_decision_level_literals_ += (state.assignments_.GetAssignmentLevel(variable) == state.GetCurrentDecisionLevel());
		++iterator;
	}

	/*std::ofstream dumperino("kekw_pseudo.txt", std::ios::app);
	dumperino << "----\n";
	dumperino << learned_pseudo_boolean_.ToString() << "\n";*/

	delete reason_constraint; //rethink this design, make it wrapped around a smart pointer? or use reference counting?
}

void ConflictAnalyserCuttingPlanes::ProcessConflictPropagator(PropagatorGeneric * conflict_propagator, BooleanLiteral propagated_literal, SolverState &state)
{
	runtime_assert(conflict_propagator != NULL);
	runtime_assert(!propagated_literal.IsUndefined() && learned_pseudo_boolean_.GetCoefficient(~propagated_literal) > 0); //todo this alters the learned constraint, might lead to different behaviour in the release version

	ReasonGeneric *reason_constraint = conflict_propagator->ReasonLiteralPropagation(propagated_literal, state); 
	reason_constraint->RoundToOne(propagated_literal, state);	

	learned_pseudo_boolean_.RoundToOne(~propagated_literal, state);
	//learned pseudo boolean has ~propagated_literal, while reason constraint has the positive version i.e. propagation literal

	//ensure the variable will be resolved by multiplying both constraints
	uint64_t coef_reason = reason_constraint->GetCoefficient(propagated_literal);
	uint64_t coef_learned = learned_pseudo_boolean_.GetCoefficient(~propagated_literal);
	uint64_t gcd = GreatestCommonDivisor(coef_learned, coef_reason);

	runtime_assert(coef_reason == 1 && coef_learned == 1); //todo this seems problematic -> since we do round to one above, their coefficients have to one...I think rounding might not be a good idea
	
	learned_pseudo_boolean_.MultiplyByFraction(coef_reason, gcd);
	reason_constraint->MultiplyByFraction(coef_learned, gcd);

	//resolve the two constraints

	//it is important to add the right hand side immediately since the methods of learned_pseudo_boolean automatically saturate after each operation
	learned_pseudo_boolean_.AddToRightHandSide(reason_constraint->GetRightHandSide());
	for (int i = 0; i < reason_constraint->Size(); i++)
	{
		Term reason_term = (*reason_constraint)[i];
		BooleanVariable reason_variable = reason_term.literal.Variable();
		
		if (state.assignments_.GetAssignmentLevel(reason_variable) == 0) //root assignment - I think this can only happen now with clausal propagators that do not remove root assignments. Not sure if I should try to remove these types of propagations at root level?
		{
			if (state.assignments_.GetAssignment(reason_term.literal) == true)
			{
				learned_pseudo_boolean_.RemoveFromRightHandSide(reason_term.coefficient);
			}
			//note hat if the literal is falsified at the root level, it is effectively not in the constraint
			continue;
		}

		runtime_assert(state.assignments_.GetAssignmentLevel(reason_variable) > 0); //these should have been removed by the propagator in the future, but for now I implemented the above hax
		runtime_assert(reason_term.coefficient != 0);
		
		learned_pseudo_boolean_.AddTerm(reason_term);
		
		//the code below deals with auxiliary data

		//when you cancel out a literal, it is not longer in the equation, but the counter is still there, I guess I should skip those that are not in
		//problem: it could be that the counter suggests there are two literals left
		//but in reality only one of them is left
		//so something needs to be done...either whenever a literal cancels, we report, or....
		//so it could be we learn something that has no literals from the current level, then it's a root assignment
		//that's a difference to SAT

		//I think the main thing here is to keep the counter of literals of the current level correct
		//for now we'll use a brute force method rather than incremental
		continue;

		runtime_assert(1 == 2);
		//do we still use this auxiliary data? Not sure...this seen was for regular conflict analysis, not sure if it applied here

		//do not update the auxiliary data if you have already seen this literal before - todo need to think about when this happens
		if (seen_[reason_variable.index_]) { continue; }

		seen_[reason_variable.index_] = true;
		encountered_variables_.push_back(reason_variable);		

		//update the counter that tracks the number of literals from the current level involved in the conflict, as detected so far
		//I think this is anyway not correct for pseudo Boolean solving
		//what's the 1uip? This code below is probably unnecessary, we'll compute the jump point later
		assert(1 == 2);
		if (state.assignments_.IsAssigned(reason_variable) )
		{
			int literal_decision_level = state.assignments_.GetAssignmentLevel(reason_variable);

			if (literal_decision_level == state.GetCurrentDecisionLevel())
			{
				num_current_decision_level_literals_++;
			}
		}
	}
	//should I saturate -> this is implicitly handled in learned_pseudo_boolean_?
	runtime_assert(propagated_literal.IsUndefined() || learned_pseudo_boolean_.GetCoefficient(~propagated_literal) == 0); //should be resolved
	runtime_assert(propagated_literal.IsUndefined() || learned_pseudo_boolean_.GetCoefficient(propagated_literal) == 0); //just a debug test, the other polarity should also not be present
	runtime_assert(!learned_pseudo_boolean_.CheckOverflow());
	//runtime_assert(learned_pseudo_boolean_.GetRightHandSide() == 1); //probably makeks sense for pure cnf, but not for MaxSAT? //I think this is just for my current tests? Since I'm giving a CNF, I can only expect these kinds of constraitns back

	//this should be done incrementally, for now we just brute force it kek
	//this is probably not even important and can be removed TODO
	num_current_decision_level_literals_ = 0;
	LearnedPseudoBooleanConstraintIterator iterator = learned_pseudo_boolean_.GetIterator();
	while (iterator.IsValid())
	{
		BooleanVariable variable = iterator.Variable();
		num_current_decision_level_literals_ += (state.assignments_.GetAssignmentLevel(variable) == state.GetCurrentDecisionLevel());
		++iterator;
	}

	/*std::ofstream dumperino("kekw_pseudo.txt", std::ios::app);
	dumperino << "----\n";
	dumperino << learned_pseudo_boolean_.ToString() << "\n";*/

	delete reason_constraint; //rethink this design, make it wrapped around a smart pointer?
}

BooleanLiteral ConflictAnalyserCuttingPlanes::FindNextReasonLiteralOnTheTrail(SolverState &state)
{
	assert(num_current_decision_level_literals_ > 0);

	//expand a node of the current decision level
	//skip literals that do not appear in the current conflict clause
	BooleanLiteral next_literal;
	do
	{
		next_literal = state.GetLiteralFromTheBackOfTheTrail(num_trail_literals_examined_);
		runtime_assert(state.assignments_.GetAssignmentLevel(next_literal.Variable()) == state.GetCurrentDecisionLevel());
		num_trail_literals_examined_++;
	} while (learned_pseudo_boolean_.GetCoefficient(~next_literal) == 0); //skip as long as the literal is not relevant for the conflict. Recall that if the negation of the literal has zero coefficient, then setting the literal to true definitely is not the cause of the conflict

	num_current_decision_level_literals_--; //we are expanding a node and therefore reducing the number of literals of the current level that are left to be considered
	return next_literal;
}

WatchedPseudoBooleanConstraint * ConflictAnalyserCuttingPlanes::AddLearnedPseudoBooleanConstraintToDatabase(LearnedPseudoBooleanConstraint &learned_constraint, SolverState &state)
{
	LearnedPseudoBooleanConstraintIterator term_iterator = learned_constraint.GetIterator();
	std::vector<BooleanLiteral> literals;
	std::vector<uint32_t> coeffs;

	while (term_iterator.IsValid())
	{
		literals.push_back(term_iterator.Literal());
		coeffs.push_back(term_iterator.Coefficient());
		++term_iterator;
	}

	WatchedPseudoBooleanConstraint * constraint = new WatchedPseudoBooleanConstraint(literals, coeffs, learned_constraint.GetRightHandSide());
	
	//todo should check if clause? I think this should get checked before calling this method?
	state.propagator_pseudo_boolean_.constraint_database_.AddPermanentConstraint(constraint);//.permanent_constraints_.push_back(constraint);

	return constraint;
}

uint64_t ConflictAnalyserCuttingPlanes::GreatestCommonDivisor(uint64_t a, uint64_t b)
{
	if (b == 0) { return a; }
	return GreatestCommonDivisor(b, a % b);
}

std::vector<BooleanLiteral> ConflictAnalyserCuttingPlanes::GetConflictingPropagation(SolverState& state)
{
	int64_t slack = ComputeSlack(state);
	
	//build a heap of terms that could contribute towards propagation
	//	only consider terms that have been assigned false, since only these assignments contributed towards the propagation
	std::vector<Term> heap_terms_;
	for (LearnedPseudoBooleanConstraintIterator iter = learned_pseudo_boolean_.GetIterator(); iter.IsValid(); ++iter)
	{
		if (state.assignments_.IsAssignedFalse(iter.Literal()) 
			&& state.assignments_.IsPropagatedAtRoot(iter.Literal()) == false)
		{
			heap_terms_.push_back(Term(iter.Literal(), iter.Coefficient()));
		}
	}
	std::make_heap(heap_terms_.begin(), heap_terms_.end(), [&state](const Term& t1, const Term& t2) { return state.assignments_.GetTrailPosition(t1.literal.Variable()) < state.assignments_.GetTrailPosition(t2.literal.Variable()); });

	//go through the assignments in order of assignment and determine when propagation would have happened
	std::vector<BooleanLiteral> conflicting_propagations;
	for(int num_terms_inspected = 0; num_terms_inspected < heap_terms_.size(); num_terms_inspected++)
	{
		//get the next candidate from the heap
		std::pop_heap(heap_terms_.begin(), heap_terms_.begin() + (heap_terms_.size() - num_terms_inspected), [&state](const Term& t1, const Term& t2) { return state.assignments_.GetTrailPosition(t1.literal.Variable()) > state.assignments_.GetTrailPosition(t2.literal.Variable()); });
		Term candidate_term = heap_terms_[heap_terms_.size() - num_terms_inspected - 1];
		
		//update slack: recall this literal has been assigned false
		slack -= candidate_term.coefficient;
		
		//check for propagation: this could possibly be done more efficiently...
		for (LearnedPseudoBooleanConstraintIterator iter = learned_pseudo_boolean_.GetIterator(); iter.IsValid(); ++iter)
		{
			//todo: I am only recording conflicting propagations. We could also record that perhaps some previous decisions can be set to propagations!
			if (iter.Coefficient() > slack && state.assignments_.IsAssignedFalse(iter.Literal()))
			{
				conflicting_propagations.push_back(iter.Literal());
			}
		}

		//we terminate as soon as we detect at least one conflicting propagation. Note that the propagation will be at the lowest decision level
		if (conflicting_propagations.size() > 0) { break; }				
	}
	
	return conflicting_propagations; //note that it could be the case that no conflicting propagations were found, in which case the empty vector is returned
}

std::vector<BooleanLiteral> ConflictAnalyserCuttingPlanes::GetNewRootPropagations(SolverState& state)
{
	std::vector<BooleanLiteral> propagated_literals;
	int64_t slack = ComputeSlack(state);
	for (LearnedPseudoBooleanConstraintIterator iter = learned_pseudo_boolean_.GetIterator(); iter.IsValid(); ++iter)
	{
		if (state.assignments_.IsPropagatedFalseAtRoot(iter.Literal())) { continue; }
		if (iter.Coefficient() > slack)
		{ 
			//I am not sure if we can learn a constraint that propagates something that is already propagated at the root
			//I think not, and for now I am just going to set a runtime assert to check
			runtime_assert(state.assignments_.IsPropagatedAtRoot(iter.Literal()) == false);
			propagated_literals.push_back(iter.Literal()); 
		}
	}
	return propagated_literals;
}

int64_t ConflictAnalyserCuttingPlanes::ComputeSlack(SolverState& state)
{
	int64_t slack = -learned_pseudo_boolean_.GetRightHandSide();
	for (LearnedPseudoBooleanConstraintIterator iter = learned_pseudo_boolean_.GetIterator(); iter.IsValid(); ++iter)
	{
		if (state.assignments_.IsPropagatedFalseAtRoot(iter.Literal())) { continue; }
		slack += iter.Coefficient();
	}
	return slack;
}

void ConflictAnalyserCuttingPlanes::ClearDataStructures()
{
	learned_pseudo_boolean_.Clear();
	num_current_decision_level_literals_ = 0; //this is anyway set to zero after conflict analysis
	num_trail_literals_examined_ = 0;

	for (BooleanVariable variable : encountered_variables_) { seen_[variable.index_] = false; }
	encountered_variables_.clear();
}

bool ConflictAnalyserCuttingPlanes::CheckDataStructures()
{
	assert(learned_pseudo_boolean_.Empty());
	assert(num_current_decision_level_literals_ == 0);
	assert(num_trail_literals_examined_ == 0);
	assert(encountered_variables_.empty());
	return true;
}

} //end Pumpkin namespace