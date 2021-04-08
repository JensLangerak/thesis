//
// Created by jens on 16-03-21.
//

#include "watched_pb_sum_constraint.h"
#include "../../../Basic Data Structures/boolean_literal.h"
#include "../../../Engine/solver_state.h"
#include "explanation_pb_sum_constraint.h"

namespace Pumpkin {
WatchedPbSumConstraint::WatchedPbSumConstraint(
    std::vector<BooleanLiteral> &inputs, std::vector<uint32_t> &input_weights,
    std::vector<BooleanLiteral> outputs, std::vector<uint32_t> &output_weights,
    IEncoder<PbSumConstraint> *encoder)
    : encoder_(encoder), true_count_(0) , debug_index(next_debug_index){
  next_debug_index++;
  assert(inputs.size() == input_weights.size());
  assert(outputs.size() == output_weights.size());
  inputs_.reserve(inputs.size());
  outputs_.reserve(outputs.size());
  for (int i = 0; i < inputs.size(); ++i) {
    inputs_.push_back(WeightedLiteral(inputs[i], input_weights[i]));
    lit_to_input_index_[inputs[i].ToPositiveInteger()] = i;
    if (input_weights[i] > max_weight_)
      max_weight_ = input_weights[i];
    lit_sum_ += input_weights[i];
  }
  for (int i = 0; i < outputs.size(); ++i) {
    outputs_.push_back(WeightedLiteral(outputs[i], output_weights[i]));
    lit_to_output_index_[outputs[i].ToPositiveInteger()] = i;
    weight_output_index_map_[output_weights[i]] = i;
    if (output_weights[i] > max_) {
      max_ = output_weights[i];
    }
  }
}

ExplanationPbSumConstraint *
WatchedPbSumConstraint::ExplainLiteralPropagation(BooleanLiteral literal,
                                                  SolverState &state) {
  return new ExplanationPbSumConstraint(this, state, literal);
}
ExplanationPbSumConstraint *
WatchedPbSumConstraint::ExplainFailure(SolverState &state) {
  return new ExplanationPbSumConstraint(this, state);
}
WatchedPbSumConstraint::~WatchedPbSumConstraint() {
  if (encoder_ != nullptr)
    delete encoder_;
}
void WatchedPbSumConstraint::UpdateCounts(std::vector<BooleanLiteral> &lits,
                                          SolverState &state) {

  propagate_count++;

  if (!encoder_->AddOnRestart())
    return;
  bool add_partial = encoder_->EncodingPartialAdded(); // TODO scheduled
  if (!add_partial) {
    int size = std::min(max_, (int) lit_sum_);
    add_partial = propagate_count > size * encoder_->add_delay;
  }
  if (add_partial) {
    if (!encoder_->EncodingAdded()) {
      state.propagator_pb_sum_.add_constraints_.push(this);
    }
  }
}

int WatchedPbSumConstraint::next_debug_index = 0;
} // namespace Pumpkin