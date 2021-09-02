﻿#include "EncoderGeneralisedTotaliserCP19_2.h"
#include "../Utilities/runtime_assert.h"

#include <set>
#include <time.h>
#include <iostream>

namespace Pumpkin
{

GeneralisedTotaliserCP192::GeneralisedTotaliserCP192() {
	backward_propagation = false;
	merge_identical_weights = false;
	special_merge_strategy = false;
	sorting_in_gte = true;
	stable_sort = false;
	has_encoded = false;

	block_sort_size = 1;
}

bool GeneralisedTotaliserCP192::ReduceRightHandSide(int64_t new_rhs)
{
	if (partial_sum_literals.empty()) //the Pseudo-Boolean constraint has not been encoded
	{
		runtime_assert(has_encoded == false);
		//add the pseudo Boolean constraint to the formula
		std::vector<BooleanLiteral> lits;
		std::vector<uint64_t> weights;

		for (PairWeightLiteral& b : objective_literals)
		{
			lits.push_back(b.literal);
			weights.push_back(b.weight);
		}
		partial_sum_literals = encode(_hax_state, lits, weights, new_rhs);
		has_encoded = true;		
		return true;
	}
	else
	{
		//encoding exists, just need to add clauses reduce the right hand side
		vector<BooleanLiteral> unit_clauses = update(partial_sum_literals, new_rhs);
		for (BooleanLiteral unit : unit_clauses)
		{
			bool ok = _hax_state->AddUnitClause(unit);
			if (!ok) { return false; }
		}
		return true;
	}
}

using namespace std;

PairWeightLiteral _global_hax_plw2(BooleanLiteral(BooleanVariable(1), true), 1);

BooleanLiteral GeneralisedTotaliserCP192::getLiteralForWeight(int weight, const vector<PairWeightLiteral> &v)
{
	_global_hax_plw2.weight = weight;

	auto iter = lower_bound(v.begin(), v.end(), _global_hax_plw2, [](const PairWeightLiteral &p1, const PairWeightLiteral &p2) {return p1.weight < p2.weight; });
	assert(iter->weight == weight);
	return iter->literal;

	//could binary search
	for (auto &p : v)
	{
		if (p.weight == weight)
		{
			return p.literal;
		}
	}
	assert(1 == 2); //should never happen
	printf("r1p\n");
	exit(1);
	return BooleanLiteral();
}

int GeneralisedTotaliserCP192::getIndexOfLiteralForWeight(int weight, const vector<PairWeightLiteral> &v)
{
	_global_hax_plw2.weight = weight;

	auto iter = lower_bound(v.begin(), v.end(), _global_hax_plw2, [](const PairWeightLiteral &p1, const PairWeightLiteral &p2) {return p1.weight < p2.weight; });
	assert(iter->weight == weight);
	return iter - v.begin();


	//could binary search
	for (int i = 0; i < v.size(); i++)
	{
		auto &p = v[i];
		if (p.weight == weight)
		{
			return i;
		}
	}
	assert(1 == 2); //should never happen
	printf("r1p index for weight %d\n", weight);
	exit(1);
	return -1;
}

vector<PairWeightLiteral> GeneralisedTotaliserCP192::create_generalised_totaliser_variables_based_on_children(vector<PairWeightLiteral> &left_child, vector<PairWeightLiteral> &right_child, int upper_bound)
{
	set<int> unique_combinations;
	//left + 0
	for (PairWeightLiteral &p : left_child)
	{
		unique_combinations.insert(p.weight);
	}
	//right + 0
	for (PairWeightLiteral &p : right_child)
	{
		unique_combinations.insert(p.weight);
	}
	//left + right
	for (PairWeightLiteral &p1 : left_child)
	{
		for (PairWeightLiteral &p2 : right_child)
		{
			unique_combinations.insert(p1.weight + p2.weight);
		}
	}
	vector<PairWeightLiteral> ret;
	for (set<int>::iterator iter = unique_combinations.begin(); iter != unique_combinations.end(); ++iter)
	{
		if (*iter > upper_bound) {
			continue;
		}

		assert(*iter > 0);

		BooleanLiteral p = BooleanLiteral(_hax_state->CreateNewVariable(), true);
		ret.push_back(PairWeightLiteral(p, *iter));
	}
	return ret;
}

vector<PairWeightLiteral> GeneralisedTotaliserCP192::generalised_totaliser_merge_two_batches(vector<PairWeightLiteral> &left_child, vector<PairWeightLiteral> &right_child, int upper_bound, bool redundant_constraints)
{
	vector<PairWeightLiteral> ret = create_generalised_totaliser_variables_based_on_children(left_child, right_child, upper_bound);

	//merge using copy-pasted old code

	//left[i] -> ret[i]
	for (PairWeightLiteral &p : left_child)
	{
		assert(p.weight <= upper_bound);	
		auto propagator = _hax_state->AddBinaryClause(~p.literal, getLiteralForWeight(p.weight, ret));
		runtime_assert(propagator == NULL); //we expect no conflicts
	}
	//right[i] -> ret[i]
	for (PairWeightLiteral &p : right_child)
	{
		assert(p.weight <= upper_bound);
		auto propagator = _hax_state->AddBinaryClause(~p.literal, getLiteralForWeight(p.weight, ret));
		runtime_assert(propagator == NULL);//we expect no conflicts
	}
	//left[i] + right[j] -> ret[i+j]
	for (PairWeightLiteral &p1 : left_child)
	{
		for (PairWeightLiteral &p2 : right_child)
		{
			int sum = p1.weight + p2.weight;

			//if the sum is within bound, enforce that its corresponding variable must activate
			//otherwise, activating the p1 and p2 should lead to an empty clause
			if (sum <= upper_bound)
			{
				auto propagator = _hax_state->AddTernaryClause(~p1.literal, ~p2.literal, getLiteralForWeight(sum, ret));
				runtime_assert(propagator == NULL);//we expect no conflicts
			}
			else
			{
				auto propagator = _hax_state->AddBinaryClause(~p1.literal, ~p2.literal);
				runtime_assert(propagator == NULL);//we expect no conflicts
			}
		}
	}

	if (redundant_constraints)
	{
		//C2(α, β, σ) = (aα + 1 ∨bβ + 1 ∨rσ + 1)

		//not left[i] -> not ret[max_right + w[left[i]]]
		for (int i = 0; i < left_child.size(); i++)
		{
			int w_prev = 0;
			if (i > 0)
			{
				w_prev = left_child[i - 1].weight;
			}

			int max_right_weight = right_child.back().weight;

			if (max_right_weight + w_prev > upper_bound)
			{
				continue;
			}


			int m = getIndexOfLiteralForWeight(max_right_weight + w_prev, ret);

			if (m + 1 >= ret.size())
			{
				continue;
			}

			auto propagator = _hax_state->AddBinaryClause(left_child[i].literal, ~ret[m+1].literal);
			runtime_assert(propagator == NULL);//we expect no conflicts
		}

		for (int i = 0; i < right_child.size(); i++)
		{
			int w_prev = 0;
			if (i > 0)
			{
				w_prev = right_child[i - 1].weight;
			}

			int max_left_weight = left_child.back().weight;

			if (max_left_weight + w_prev > upper_bound)
			{
				continue;
			}


			int m = getIndexOfLiteralForWeight(max_left_weight + w_prev, ret);

			if (m + 1 >= ret.size())
			{
				continue;
			}

			auto propagator = _hax_state->AddBinaryClause(right_child[i].literal, ~ret[m + 1].literal);
			runtime_assert(propagator == NULL);//we expect no conflicts
		}

		//not left[i] and not right[j] -> not ret[w[i-1]+w[j-1]+1]
		for (int i = 0; i < left_child.size(); i++)
		{
			for (int j = 0; j < right_child.size(); j++)
			{
				int w_prev_left = 0;
				int w_prev_right = 0;

				if (i > 0) 
				{
					w_prev_left = left_child[i - 1].weight;
				}

				if (j > 0)
				{
					w_prev_right = right_child[j - 1].weight;
				}

				if (w_prev_left + w_prev_right > upper_bound)
				{
					continue;
				}

				if (w_prev_left == 0 && w_prev_right == 0)
				{
					auto propagator = _hax_state->AddTernaryClause(left_child[i].literal, right_child[j].literal, ~ret[0].literal);
					runtime_assert(propagator == NULL);//we expect no conflicts
					continue;
				}

				int m = getIndexOfLiteralForWeight(w_prev_left + w_prev_right, ret);

				if (m + 1 >= ret.size())
				{
					continue;
				}

				auto propagator = _hax_state->AddTernaryClause(left_child[i].literal, right_child[j].literal, ~ret[m + 1].literal);
				runtime_assert(propagator == NULL);//we expect no conflicts
			}
		}
	}

	return ret;
}

vector<PairWeightLiteral> GeneralisedTotaliserCP192::generalised_totaliser_bottom_up_tree(const vector<vector<PairWeightLiteral> > &leafs, int upper_bound, bool redundant_constraints)
{
	assert(leafs.size() >= 1);

	if (special_merge_strategy == false)
	{
		vector<vector<PairWeightLiteral> > batch1, batch2;
		vector<vector<PairWeightLiteral> > *current_batch, *new_batch;

		batch1 = leafs;
		current_batch = &batch1;
		new_batch = &batch2;

		while (current_batch->size() != 1)
		{
			new_batch->resize(0);
			for (unsigned int i = 0; i < current_batch->size() / 2; i++)
			{
				vector<PairWeightLiteral> &b1 = current_batch->at(2 * i);
				vector<PairWeightLiteral> &b2 = current_batch->at(2 * i + 1);
				vector<PairWeightLiteral> merged_batch = generalised_totaliser_merge_two_batches(b1, b2, upper_bound, redundant_constraints);

				new_batch->push_back(merged_batch);
			}

			if (current_batch->size() % 2 == 1)
			{
				new_batch->push_back(current_batch->back());
			}

			vector<vector<PairWeightLiteral> >  *temp(current_batch);
			current_batch = new_batch;
			new_batch = temp;
		}
		assert(current_batch->size() == 1);
		return current_batch->at(0);
	}
	else {
		runtime_assert(1 == 2);
	}
}

//<= rhs constraint 
vector<PairWeightLiteral> GeneralisedTotaliserCP192::encode(SolverState *state, vector<BooleanLiteral> &lits, vector<uint64_t> &coeffs, uint64_t rhs)
{
	runtime_assert(lits.size() > 0);
	runtime_assert(lits.size() == coeffs.size());

	time_t start_time;
	time(&start_time);

	vector<vector<PairWeightLiteral> > relevant_variables;

	for (int i = 0; i < lits.size(); i++)
	{
		if (coeffs[i] == 0)
		{
			cout << "zero lol\n";
			continue;
		}
		else if (coeffs[i] > rhs)
		{
			state->AddUnitClause(~lits[i]);
		}
		else
		{
			PairWeightLiteral p;
			p.literal = lits[i];
			p.weight = coeffs[i];

			vector<PairWeightLiteral> v;
			v.push_back(p);

			relevant_variables.push_back(v);
		}
	}

	if (sorting_in_gte)
	{
		printf("c sorting!\n");
		if (block_sort_size == 1)
		{
			if (!stable_sort)
			{
				sort(relevant_variables.begin(), relevant_variables.end(), [](const vector<PairWeightLiteral> &p1, const vector<PairWeightLiteral> &p2) {return p1[0].weight < p2[0].weight; });
			}
			else
			{
				std::stable_sort(relevant_variables.begin(), relevant_variables.end(), [](const vector<PairWeightLiteral> &p1, const vector<PairWeightLiteral> &p2) {return p1[0].weight < p2[0].weight; });
			}
		}
		else {
			printf("c block sort!\n");
			int block_size = relevant_variables.size() / block_sort_size;
			for (int i = 0; i < block_sort_size; i++)
			{
				int start = i * block_size;
				int end = (i + 1)*block_size;

				if (i == block_sort_size - 1)
				{
					end = relevant_variables.size();
				}

				if (!stable_sort)
				{
					sort(relevant_variables.begin()+start, relevant_variables.begin()+end, [](const vector<PairWeightLiteral> &p1, const vector<PairWeightLiteral> &p2) {return p1[0].weight < p2[0].weight; });
				}
				else
				{
					std::stable_sort(relevant_variables.begin()+start, relevant_variables.begin()+end, [](const vector<PairWeightLiteral> &p1, const vector<PairWeightLiteral> &p2) {return p1[0].weight < p2[0].weight; });
				}
			}


		}

		time_t end_time;
		time(&end_time);
		double seconds = difftime(start_time, end_time);
		printf("c sorting took about %f\n", seconds);



	}
	if (merge_identical_weights == 1)
	{
		set<uint64_t> unique_values;
		for (auto &m : relevant_variables)
		{
			unique_values.insert(m[0].weight);
		}

		set<uint64_t> used_values;
		vector<uint64_t> pattern;

		for (auto iter = unique_values.begin(); iter != unique_values.end(); ++iter)
		{
			uint64_t val = *iter;
			while (unique_values.find(val) != unique_values.end() && used_values.find(val) == used_values.end())
			{
				pattern.push_back(val);
				used_values.insert(val);
				val *= 2;
			}
		}
		vector<vector<PairWeightLiteral> > reordered_variables;
		for (int i = 0; i < pattern.size(); i++)
		{
			vector<PairWeightLiteral> temp;
			PairWeightLiteral temp2(BooleanLiteral(BooleanVariable(1), true), pattern[i]);
			temp.push_back(temp2);
			auto iter = lower_bound(relevant_variables.begin(), relevant_variables.end(), temp, [](const vector<PairWeightLiteral> &p1, const vector<PairWeightLiteral> &p2) { return p1[0].weight < p2[0].weight; });

			while (iter != relevant_variables.end() && iter->at(0).weight == pattern[i])
			{
				reordered_variables.push_back(*iter);
				++iter;
			}
		}
		relevant_variables = reordered_variables;

	}


	if (merge_identical_weights == 2)
	{
		set<uint64_t> unique_values;
		for (auto &m : relevant_variables)
		{
			unique_values.insert(m[0].weight);
		}

		set<uint64_t> used_values;
		vector<uint64_t> pattern;

		for (auto iter = unique_values.begin(); iter != unique_values.end(); ++iter)
		{
			uint64_t val = *iter;
			if (used_values.find(2*val) == used_values.end())
			{
				pattern.push_back(val);
				pattern.push_back(2*val);
				used_values.insert(val);
				used_values.insert(2 * val);
			}
		}

		vector<vector<PairWeightLiteral> > reordered_variables;
		for (int i = 0; i < pattern.size(); i++)
		{
			vector<PairWeightLiteral> temp;
			PairWeightLiteral temp2(BooleanLiteral(BooleanVariable(1), true), pattern[i]);
			temp.push_back(temp2);
			auto iter = lower_bound(relevant_variables.begin(), relevant_variables.end(), temp, [](const vector<PairWeightLiteral> &p1, const vector<PairWeightLiteral> &p2) { return p1[0].weight < p2[0].weight; });

			while (iter != relevant_variables.end() && iter->at(0).weight == pattern[i])
			{
				reordered_variables.push_back(*iter);
				++iter;
			}
		}
		relevant_variables = reordered_variables;		
	}

	if (merge_identical_weights == 3)
	{
		assert(relevant_variables[0].size() == 1);
		vector<vector<PairWeightLiteral> > merged_variables;
		int i = 0;
		int current_weight = relevant_variables[0][0].weight;
		vector<vector<PairWeightLiteral> > vars_in_block;
		while (i < relevant_variables.size())
		{
			if (relevant_variables[i][0].weight != current_weight)
			{
				cout << "c merging " << vars_in_block.size() << " of " << current_weight << endl;
				merged_variables.push_back(generalised_totaliser_bottom_up_tree(vars_in_block, rhs, this->backward_propagation));
				cout << "c \t" << merged_variables.back().size() << endl;
				vars_in_block.clear();
				current_weight = relevant_variables[i][0].weight;
			}
			vector<PairWeightLiteral> hehe;
			hehe.push_back(relevant_variables[i][0]);
			vars_in_block.push_back(hehe);
			i++;
		}		
		merged_variables.push_back(generalised_totaliser_bottom_up_tree(relevant_variables, rhs, this->backward_propagation));

		relevant_variables = merged_variables;
	}

	std::vector<PairWeightLiteral> _output_literals;
	
	if (relevant_variables.size() >= 1 )
		_output_literals = generalised_totaliser_bottom_up_tree(relevant_variables, rhs, this->backward_propagation);

	time_t end_time;
	time(&end_time);
	double seconds = difftime(start_time, end_time);
	printf("c gte took about %f\n", seconds);

	//for (auto iter = _output_literals.begin(); iter != _output_literals.end(); ++iter)
	//{
		//cout << "c " << iter->weight << " ";
	//}
	//cout << endl;
	return _output_literals;
}

//<= rhs
vector<BooleanLiteral> GeneralisedTotaliserCP192::update(vector<PairWeightLiteral> &output_literals, uint64_t rhs)
{
	vector<BooleanLiteral> ret;
	for (auto iter = output_literals.rbegin(); iter != output_literals.rend(); ++iter)
	{
		if (iter->weight > rhs)
		{
			ret.push_back(~iter->literal);
		}
		else {
			break;
		}
	}
	return ret;
}

} //end Pumpkin namespace