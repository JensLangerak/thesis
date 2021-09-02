#pragma once

#include "../Engine/solver_state.h"
#include "../Utilities/boolean_literal.h"
#include "../Utilities/boolean_assignment_vector.h"
#include "../Utilities/pair_weight_literal.h"
#include "../Utilities/directly_hashed_integer_set.h"

#include <vector>
#include <unordered_map>
#include <set>

namespace Pumpkin
{
class EncoderGeneralisedTotaliser
{
public:
	//encodes the soft constraint \sum w_i * x_i <= right_hand_side
	//the output literals indicate the degree of violation of the constraint, i.e., output[i].literal = true if the sum is output[i].weight units over right_hand_side 
	std::vector<PairWeightLiteral> SoftLessOrEqual(std::vector<PairWeightLiteral>& input_literals, int right_hand_side, SolverState& state);

	//encodes the hard constraint \sum w_i * x_i <= right_hand_side
	//the output literals indicate the sum of the variables up to (and including) the right_hand_side, i.e., if the sum is at least k, then the sum of the first m output variables that sum up to k will be set to true
	std::vector<PairWeightLiteral> HardLessOrEqual(std::vector<PairWeightLiteral>& input_literals, int right_hand_side, SolverState& state);

	void Clear(); //removes info about encoded constraints

	bool DebugCheckSatisfactionOfEncodedConstraints(BooleanAssignmentVector& solution);

	std::vector<std::vector<PairWeightLiteral> > literals_from_last_hard_call_; //todo this is a bit hacky now, improve. This stores all variables used the last time the hard upper bound constraint was encoded (used in linear search). These variables are used for value selection if optimistic-aux is selected.

private:
	struct Layer
	{
		std::vector<PairWeightLiteral> pairs;
		std::vector<size_t> node_start; // [i] provides the index of the first literal of the i-th node in the pairs vector.
		
		size_t NumNodes() const { return node_start.size() - 1; }
		size_t NodeStart(size_t node_index) const { return node_start[node_index]; }
		size_t NodeEnd(size_t node_index) const { return node_start[node_index + 1]; }
		size_t NodeSize(size_t node_index) { return node_start[node_index + 1] - node_start[node_index]; } //the last entry of node_start is a dummy node, used only to help with this calculation
		size_t NumLiterals() const { return pairs.size(); }
		
		void InitialiseLayer(std::vector<PairWeightLiteral>& p)
		{
			pairs = p;
			node_start.resize(pairs.size() + 1);
			for (size_t i = 0; i < pairs.size() + 1; i++) { node_start[i] = i; }
		}

		void Clear() { pairs.clear(); node_start.clear(); node_start.push_back(0); }
		void AddNode() { node_start.push_back(node_start.back()); } //just add an empty node
	};

	std::vector<Layer> debug_last_call_;	

	Layer helper1_, helper2_;
	std::unordered_map<int64_t, size_t> value_to_index_map_;
	std::set<int64_t> partial_sums_;
	//DirectlyHashedIntegerSet value_to_index_map_;

	struct EncodedConstraint { std::vector<PairWeightLiteral> input_literals, output_literals; int64_t right_hand_side; };
	std::vector<EncodedConstraint> encoded_soft_constraints_;
	std::vector<EncodedConstraint> encoded_hard_constraints_;
};
}