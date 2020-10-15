#ifndef SMALL_HELPER_STRUCTURES_H
#define SMALL_HELPER_STRUCTURES_H

#include "boolean_literal.h"

#include <vector>
#include <assert.h>
#include <math.h>

namespace Pumpkin
{

class TwoWatchedClause; //circular dependency?
class PropagatorGeneric;

typedef std::vector<BooleanLiteral> Disjunction; //mainly to increase readability, i.e. to emphasize what the vector represents //todo previously there was a "Conjunction" class but I removed it, not sure if I should still keep Disjunction

struct Term
{
	Term() {};
	Term(const BooleanLiteral lit, uint32_t coef) :literal(lit), coefficient(coef) {}

	BooleanVariable Variable() const { return literal.Variable(); }

	BooleanLiteral literal;
	uint32_t coefficient;
};

/*struct Term64
{
	Term64(BooleanLiteral lit, uint64_t coef) :literal(lit), coefficient64(coef) {}
	BooleanLiteral literal;
	uint64_t coefficient64;
};*/

struct ConflictAnalysisResultClausal
{
	ConflictAnalysisResultClausal(Disjunction &d, BooleanLiteral lit, int b) :learned_clause_literals(d), propagated_literal(lit), backtrack_level(b) {}

	Disjunction learned_clause_literals;
	BooleanLiteral propagated_literal;
	int backtrack_level;

	/*
	LearnedPseudoBooleanConstraint learned_pseudo_boolean;
	std::vector<BooleanLiteral> forced_assignments_at_root_node; //todo: I think I want to remove this. If there are root assignments, then the backtrack level could be set to zero
	std::vector<BooleanLiteral> propagations_at_backtrack_level;
	int backtrack_level;
	*/
};

struct PropagationResult
{
	PropagationResult(BooleanLiteral propagation_literal, BooleanLiteral new_watched_literal) :propagated_literal(propagation_literal), new_watched_literal(new_watched_literal) {}
	BooleanLiteral propagated_literal, new_watched_literal;
};

struct WatchedLiterals
{
	WatchedLiterals(BooleanLiteral l1, BooleanLiteral l2) :w1(l1), w2(l2) {}
	BooleanLiteral w1, w2;
};

struct Assignment_old
{
	Assignment_old(bool truth_value, int decision_level, TwoWatchedClause * reason) :truth_value(truth_value), decision_level(decision_level), reason(reason) {}
	bool truth_value;
	int decision_level;
	TwoWatchedClause *reason;	
};

struct AuxiliaryAssignmentInfo
{
	AuxiliaryAssignmentInfo() :responsible_propagator(NULL), code(0), decision_level(-1), position_on_trail(-1) {}
	PropagatorGeneric *responsible_propagator;
	uint64_t code; //this should be a piece of information that the propagator will decode and use to help it reconstruct the reason of the propagation
	int decision_level;
	int position_on_trail;

	void Clear() { responsible_propagator = NULL; code = 0; decision_level = -1; position_on_trail = -1; }
};

struct PairTimeCost
{
	PairTimeCost(time_t time, long long cost) :time(time), cost(cost) {}

	time_t time;
	long long cost;
};

class TimeStamps
{
public:
	TimeStamps() {}

	void AddTimePoint(time_t time, long long cost)
	{
		assert(time_points_.empty() || time_points_.back().time <= time && time_points_.back().cost >= cost);
		time_points_.push_back(PairTimeCost(time, cost));
	}

	//computes the integral of the curve defined by the time points (time, cost)
	//there is an undefined case when computing the integral from time zero to the time the first solution is computed
	//we are assuming the initial solution can be computed quickly
	//therefore we will use the score of the first solution found when there is no solution present
	//might be problematic if only one solution was computed or when it takes a long time to compute the initial solution, but this is more of an exception
	double ComputePrimalIntegral(time_t end_time) const
	{
		if (time_points_.size() == 0) { return INFINITY; }

		if (time_points_.size() == 1) { return (double(end_time) * time_points_[0].cost); }

		double integral = 0.0;
		for (unsigned int i = 1; i < time_points_.size(); i++)
		{
			time_t delta_t = time_points_[i].time - time_points_[i - 1].time;
			integral += (delta_t * time_points_[i - 1].cost);
		}
		integral += (end_time - time_points_.back().time)*time_points_.back().cost;
		return integral;
	}

private:
	std::vector<PairTimeCost> time_points_;
};

struct PairUnsatFlagPropagationLiterals
{
	PairUnsatFlagPropagationLiterals(bool unsat, const std::vector<BooleanLiteral> &propagation_lits) :
		is_unsat(unsat), propagation_literals(propagation_lits) {}

	bool is_unsat;
	std::vector<BooleanLiteral> propagation_literals;
};

} //end Pumpkin namespace

#endif // !SMALL_HELPER_STRUCTURES_H