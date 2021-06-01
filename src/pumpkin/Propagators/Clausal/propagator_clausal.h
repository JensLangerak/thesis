#pragma once

#include "two_watched_clause.h"
#include "clause_database.h"
#include "explanation_clausal.h"
#include "../propagator_generic.h"
#include "../explanation_generic.h"
#include "../explanation_generator.h"
#include "../../Utilities/small_helper_structures.h"

namespace Pumpkin
{

class PropagatorClausal : public PropagatorGeneric
{
public:
	PropagatorClausal
	(
		int64_t num_variables, 
		double decay_factor,
		int lbd_threshold,
		int target_max_num_temporary_clauses,
		bool use_LBD_for_sorting_temporary_clauses
	);

	void Synchronise(SolverState& state);

	ExplanationGeneric * ExplainLiteralPropagation(BooleanLiteral literal, SolverState &state); //returns the conjunction that forces the assignment of input literal to true. Assumes the input literal is not undefined.
	ExplanationGeneric * ExplainFailure(SolverState &state); //returns the conjunction that leads to failure

	ReasonGeneric * ReasonLiteralPropagation(BooleanLiteral literal, SolverState &state);
	ReasonGeneric * ReasonFailure(SolverState &state);

	//assumes that before calling this method, the solver state has completed propagation, i.e., there are no literals waiting in the propagation queue
	//adds a clause to the database and performs propagation 
	//returns the conflicting propagator if a conflict occured, otherwise returns NULL if no conflicts happened
	//if a conflicted took place, conflict analysis should be performed using the propagator to restore the solver to a non-conflicting state or report unsatisfiability
	PropagatorGeneric* AddPermanentClause(std::vector<BooleanLiteral>& literals, SolverState& state);

	ClauseDatabase clause_database_;

	//assumes the propagator detected a conflict/failure clause
	//calls BumpClauseActivityAndUpdateLBD with the failure clause
	void BumpFailureClause(SolverState&); 
	//assumes the propagator propagated the literal
	//calls BumpClauseActivityAndUpdateLBD with the clause that propagated the literal
	void BumpPropagatingClause(BooleanLiteral propagated_literal, SolverState&);
        void GrowDatabase() override;

      private:
	//if the clause is a learned clause, increases its activity and updates its LBD score
	//for nonlearned/permanent clauses, this method does nothing
	void BumpClauseActivityAndUpdateLBD(TwoWatchedClause*, SolverState&);

	//this is the main propagation method. Note that it will change watch lists of true_literal and some other literals and enqueue assignments
	bool PropagateLiteral(BooleanLiteral true_literal, SolverState &state);	

	TwoWatchedClause *failure_clause_;
	ExplanationGenerator<ExplanationClausal> explanation_generator_;
};

} //end Pumpkin namespace
