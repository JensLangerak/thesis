#ifndef PROPAGATOR_GENERIC_H
#define PROPAGATOR_GENERIC_H

#include "explanation_generic.h"
#include "reason_generic.h"
#include "../Basic Data Structures/trail_list.h"
#include <list>

namespace Pumpkin
{

class SolverState;

class PropagatorGeneric
{
public:
	PropagatorGeneric();

	bool Propagate(SolverState &state); //does full propagation, i.e. until there is nothing else left to propagate
	virtual bool PropagateOneLiteral(SolverState &state); //does only a single literal propagation, which is useful since it allows us to then ask simpler propagators to propagate with respect to the new enqueued literal before going further with this propagator
	virtual void Synchronise(SolverState &state); //after the state backtracks, it should call this synchronise method which will internally set the pointer of the trail to the new correct position

	virtual ExplanationGeneric * ExplainLiteralPropagation(BooleanLiteral literal, SolverState &state) = 0; //returns the explanation of the propagation. Assumes the input literal is not undefined.
	virtual ExplanationGeneric * ExplainFailure(SolverState &state) = 0; //returns the explanation of the failure

	virtual ReasonGeneric * ReasonLiteralPropagation(BooleanLiteral literal, SolverState &state) = 0;
	virtual ReasonGeneric * ReasonFailure(SolverState &state) = 0;
	
	bool IsPropagationComplete(SolverState &state);

        inline virtual void SetTrailIterator(TrailList<BooleanLiteral>::Iterator iterator) { next_position_on_trail_to_propagate_it = iterator;}
        bool CheckTrailIterator(SolverState &state);

//protected:
	//this is the main propagation method. Note that it will change watch lists of true_literal and some other literals and enqueue assignments
	virtual bool PropagateLiteral(BooleanLiteral true_literal, SolverState &state) = 0;

	BooleanLiteral GetAndPopNextLiteralToPropagate(SolverState &state);
	
	//tracks the position of the literals on the trail that needs to be propagated
	//needs to be updated each Backtrack using Synchronise
//	size_t next_position_on_trail_to_propagate_;
        TrailList<BooleanLiteral>::Iterator next_position_on_trail_to_propagate_it;
};

} //end Pumpkin namespace

#endif // !PROPAGATOR_GENERIC_H
