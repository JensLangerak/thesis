#ifndef ENCODER_TOTALISER_CP19_2
#define ENCODER_TOTALISER_CP19_2

#include "../Utilities/boolean_literal.h"
#include "../Utilities/problem_specification.h"
#include "../Engine/solver_state.h"

#include <map>
#include <utility>
#include <vector>
#include <algorithm>
#include <assert.h>

using namespace std;

namespace Pumpkin
{

	typedef std::pair<std::vector<std::vector<int> >, std::vector<std::vector<int> > > hard_soft_clauses;
	typedef vector<int> clause;
	
	class GeneralisedTotaliserCP192{

	public:

		bool backward_propagation;
		int merge_identical_weights;
		bool special_merge_strategy;
		bool sorting_in_gte;
		bool stable_sort;
		int block_sort_size;
		bool has_encoded;
		std::vector<PairWeightLiteral> objective_literals;
		std::vector<PairWeightLiteral> partial_sum_literals;

		GeneralisedTotaliserCP192();
		~GeneralisedTotaliserCP192() {}

		bool ReduceRightHandSide(int64_t new_rhs);

		// EncodeSoftConstraint constraint.
		vector<PairWeightLiteral> encode(SolverState *state, std::vector<BooleanLiteral> &lits, std::vector<uint64_t> &coeffs, uint64_t rhs);
		vector<BooleanLiteral> update(vector<PairWeightLiteral> &output_variables, uint64_t rhs);

		BooleanLiteral getLiteralForWeight(int weight, const vector<PairWeightLiteral> &v);
		int getIndexOfLiteralForWeight(int weight, const vector<PairWeightLiteral> &v);

		vector<PairWeightLiteral> generalised_totaliser_bottom_up_tree(const vector<vector<PairWeightLiteral> >  &leafs, int upper_bound, bool redundant_constraints);
		vector<PairWeightLiteral> create_generalised_totaliser_variables_based_on_children(vector<PairWeightLiteral> &left_child, vector<PairWeightLiteral> &right_child, int upper_bound);
		vector<PairWeightLiteral> generalised_totaliser_merge_two_batches(vector<PairWeightLiteral> &left_child, vector<PairWeightLiteral> &right_child, int upper_bound, bool redundant_constraints);
	
		SolverState * _hax_state;
	};

} //end Pumpkin namespace

#endif // !ENCODER_TOTALISER_CP19
