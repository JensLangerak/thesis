//
// Created by jens on 16-03-21.
//

#include "generalized_totaliser_sum_nodes.h"
#include "../../../Engine/solver_state.h"
#include "generalized_totaliser_sum_helpers.h"
#include <algorithm>
#include <iostream>
#include <stack>
#include <stdlib.h>
#include <utility>
#include "../Sum/watched_pb_sum_constraint.h"
namespace Pumpkin {

void GeneralizedTotaliserSumNodes::Encode(SolverState &state,
                                     std::vector<BooleanLiteral> lits) {
  assert(false);
}
void GeneralizedTotaliserSumNodes::Encode(SolverState &state) {
  if (encoding_added_) {
    return;
  }
//  std::cout << "Encode " << inputs_.size() << std::endl;

  if (inputs_.size() <= 2) {
    CreateLeafs(state);
    return;
  }
  // create two groups.
  std::vector<LitActiveData> lits;
  lits.reserve(inputs_.size());

  for (WeightedLiteral wl : inputs_) {
    double a = state.variable_selector_.heap_.GetKeyValue(wl.literal.VariableIndex() - 1);
    lits.push_back(LitActiveData(wl.literal, a, wl.weight));
  }
  std::sort(lits.begin(), lits.end(), [](LitActiveData a, LitActiveData b) {
    if (a.weight == b.weight)
      return a.activity < b.activity;
    return a.weight < b.weight;
  }); // TODO order
//  std::sort(lits.begin(), lits.end(), [](LitActiveData a, LitActiveData b) {
//    if ((a.activity != 0 && abs(a.activity - b.activity) / a.activity > 0.1O) || (a.activity == 0 && b.activity > 0.01))
//      return a.activity < b.activity;
//    return a.weight < b.weight;
//  }); // TODO order


  int halfway = lits.size() / 2;
  int start = halfway - (lits.size() / 10);
  int end = halfway + (lits.size() / 10) + 1;
  //TODO
//  int start = halfway;
//  int end = halfway + 1;
  end = std::min(end, (int) lits.size());
  start = std::max(start, 1);
  int best_gap = 0;
  int best_index = start;
  for (int i = start; i < end; ++i) {
    int diff = lits[i].activity - lits[i-1].activity;
    int scaled_diff = diff / lits[i-1].activity;
    if (scaled_diff > best_gap) {
      best_gap = scaled_diff;
      best_index = i;
    }
  }
  std::vector<WeightedLiteral> l_input;
  for (int i = 0; i < best_index; ++i) {
    l_input.push_back(WeightedLiteral(lits[i].literal, lits[i].weight));
  }
  std::vector<WeightedLiteral> r_input;
  for (int i = best_index; i < lits.size(); ++i) {
    r_input.push_back(WeightedLiteral(lits[i].literal, lits[i].weight));
  }
  assert(l_input.size() + r_input.size() == inputs_.size());
  int diff = l_input.size() - r_input.size();
//  assert(abs(diff) < 2);

  std::vector<WeightedLiteral> l_sum_literals = GeneralizedTotaliserSumHelpers::CreateSumLiterals(l_input, max_, state);
  std::vector<WeightedLiteral> r_sum_literals = GeneralizedTotaliserSumHelpers::CreateSumLiterals(r_input, max_, state);

  auto l_constraint = GeneralizedTotaliserSumHelpers::AddPbSumConstraint(l_input, l_sum_literals,max_, add_delay, encoding_strategy_, state, l_child);
  auto r_constraint = GeneralizedTotaliserSumHelpers::AddPbSumConstraint(r_input, r_sum_literals,max_, add_delay, encoding_strategy_, state, r_child);

  if (encoding_strategy_ != EncodingStrategy::INCREMENTAL){
    l_constraint->encoder_->Encode(state);
    r_constraint->encoder_->Encode(state);
  }

  // TODO max weight check and violation
  for (WeightedLiteral l : l_sum_literals) {
    std::vector<BooleanLiteral> clause = {~(l.literal), GetLiteral(l.weight).literal};
    state.propagator_clausal_.clause_database_.AddPermanentClause(clause, state);
  }
  for (WeightedLiteral r : r_sum_literals) {
    std::vector<BooleanLiteral> clause = {~(r.literal), GetLiteral(r.weight).literal};
    state.propagator_clausal_.clause_database_.AddPermanentClause(clause, state);
  }
  int ls = l_sum_literals.size();
  int rs = r_sum_literals.size();
  int upper = ls * rs;
  for (WeightedLiteral l : l_sum_literals) {
    for (WeightedLiteral r : r_sum_literals) {
      int w = l.weight + r.weight;
      if (w > max_) {
        std::vector<BooleanLiteral> clause = {~(l.literal), ~(r.literal)};
        state.propagator_clausal_.clause_database_.AddPermanentClause(clause,
                                                                      state);
      } else {
        std::vector<BooleanLiteral> clause = {~(l.literal), ~(r.literal),
                                              GetLiteral(w).literal};
        state.propagator_clausal_.clause_database_.AddPermanentClause(clause,
                                                                      state);
      }
    }
  }
  encoding_added_ = true;
}
WeightedLiteral GeneralizedTotaliserSumNodes::GetLiteral(int weight) {
  if (weight > max_)
    weight = lowest_after_max_;

  assert(weight_map_.count(weight) != 0);
  WeightedLiteral res = weight_map_[weight];
  assert(res.weight == weight || (res.weight == lowest_after_max_ && weight > lowest_after_max_));
  return weight_map_[weight];
}

GeneralizedTotaliserSumNodes::GeneralizedTotaliserSumNodes(
    std::vector<WeightedLiteral> inputs, std::vector<WeightedLiteral> outputs, int max) : inputs_(std::move(inputs)), outputs_(outputs), max_(max),  lowest_after_max_(INT32_MAX), l_child(nullptr), r_child(nullptr){
  lowest_after_max_ = -1;
  max_output_ = -1;
  for (WeightedLiteral w : outputs) {
    weight_map_[w.weight] = w;
    if (w.weight > max && (w.weight < lowest_after_max_ || lowest_after_max_ == -1))
      lowest_after_max_ = w.weight;
    if (max_output_ < w.weight)
      max_output_ = w.weight;
  }
  if (lowest_after_max_ == -1)
    lowest_after_max_ = max;
}
bool GeneralizedTotaliserSumNodes::UpdateMax(int max, SolverState &state) {
  if (max >= max_output_)
    return true;
  assert(max < max_);
  for (WeightedLiteral w : outputs_) {
    if (w.weight > max && w.weight <= this->max_) {
      BooleanLiteral l = ~(w.literal);
      state.AddUnitClause(l);
      if (w.weight < lowest_after_max_)
        lowest_after_max_ = w.weight;
    }
  }

  this->max_ = max;
  this->watched_constraint->current_max_ = max;

  bool res = true;
  if (r_child != nullptr)
    res = res & r_child->UpdateMax(max, state);
  if (l_child != nullptr)
    res = res & l_child->UpdateMax(max, state);
  return res;

}
void GeneralizedTotaliserSumNodes::CreateLeafs(SolverState &state) {

  assert(inputs_.size() <= 2);
  if (inputs_.empty())
    return;

  int sum = 0;
  for (WeightedLiteral l : inputs_) {
    std::vector<BooleanLiteral> clause = {~(l.literal), GetLiteral(l.weight).literal};
    state.propagator_clausal_.clause_database_.AddPermanentClause(clause, state);
    sum+=l.weight;
  }

  if (inputs_.size() == 2) {
      WeightedLiteral l = inputs_[0];
      WeightedLiteral r = inputs_[1];
      std::vector<BooleanLiteral> clause = {~(l.literal), ~(r.literal), GetLiteral(sum).literal};
      state.propagator_clausal_.clause_database_.AddPermanentClause(clause, state);
  }

  encoding_added_ = true;

}

IEncoder<PbSumConstraint> *
GeneralizedTotaliserSumNodes::Factory::CallConstructor(
    PbSumConstraint &constraint) {
  std::vector<WeightedLiteral> inputs;
  std::vector<WeightedLiteral> outputs;
  outputs.reserve(constraint.output_literals.size());
  int max = 0;

  for (int i = 0; i < constraint.output_literals.size(); ++i) {
    outputs.push_back(WeightedLiteral(constraint.output_literals[i], constraint.output_coefficients[i]));
    if (constraint.output_coefficients[i] > max)
      max = constraint.output_coefficients[i];
  }
  inputs.reserve(constraint.input_literals.size());
  for (int i = 0; i < constraint.input_literals.size(); ++i) {
    inputs.push_back(WeightedLiteral(constraint.input_literals[i], constraint.input_coefficients[i]));
  }
  GeneralizedTotaliserSumNodes * res = new GeneralizedTotaliserSumNodes(inputs, outputs, max);
  return res;
}
}