//
// Created by jens on 18-06-21.
//

#include "watched_pb_sum_constraint.h"
#include "../Encoders/i_encoder.h"
#include "../../../Engine/solver_state.h"
#include "Input/explanation_pb_sum_constraint_input.h"
#include "Output/explanation_pb_sum_constraint_output.h"

namespace Pumpkin {

WatchedPbSumConstraint::WatchedPbSumConstraint(
    PbSumConstraint constraint, IEncoder<PbSumConstraint>::IFactory *encoder_factory)
    : encoder_(encoder_factory->Create(constraint)) {
  auto inputs = constraint.input_literals;
  auto input_weights = constraint.input_coefficients;
  auto outputs = constraint.output_literals;
  auto output_weights = constraint.output_coefficients;
  assert(inputs.size() == input_weights.size());
  assert(outputs.size() == output_weights.size());
  inputs_.reserve(inputs.size());
  outputs_.reserve(outputs.size());
  for (int i = 0; i < inputs.size(); ++i) {
    inputs_.push_back(WeightedLiteral(inputs[i], input_weights[i]));
    lit_to_input_index_[inputs[i]] = i;
    if (input_weights[i] > max_weight_)
      max_weight_ = input_weights[i];
    lit_sum_ += input_weights[i];
  }
  for (int i = 0; i < outputs.size(); ++i) {
    outputs_.push_back(WeightedLiteral(outputs[i], output_weights[i]));
    lit_to_output_index_[outputs[i]] = i;
    weight_output_index_map_[output_weights[i]] = i;
    if (output_weights[i] > max_) {
      max_ = output_weights[i];
    }
  }
  current_max_ = max_;
}
ExplanationGeneric *WatchedPbSumConstraint::ExplainLiteralPropagationInput(
    BooleanLiteral literal, SolverState &state,
    ExplanationDynamicConstraint *explanation) {
  ExplanationPbSumConstraintInput::InitExplanationPbSumInput(
      this, state, literal, explanation);
  return explanation;
}
ExplanationGeneric *WatchedPbSumConstraint::ExplainFailureInput(
    SolverState &state, ExplanationDynamicConstraint *explanation) {
  ExplanationPbSumConstraintInput::InitExplanationPbSumInput(this, state,
                                                             explanation);
  return explanation;
}
ExplanationGeneric *WatchedPbSumConstraint::ExplainLiteralPropagationOutput(
    BooleanLiteral literal, SolverState &state,
    ExplanationDynamicConstraint *explanation) {
  ExplanationPbSumConstraintOutput::InitExplanationPbSumOutput(
      this, state, literal, explanation);
  return explanation;
}
ExplanationGeneric *WatchedPbSumConstraint::ExplainFailureOutput(
    SolverState &state, ExplanationDynamicConstraint *explanation) {
  ExplanationPbSumConstraintOutput::InitExplanationPbSumOutput(this, state,
                                                               explanation);
  return explanation;
}
void WatchedPbSumConstraint::AddScheduledEncoding(SolverState &state) {
  if (encoder_->EncodingAdded())
    return;
  encoder_->Encode(state);
}
ExplanationGeneric *WatchedPbSumConstraint::ExplainLiteralPropagation(
    BooleanLiteral literal, SolverState &state,
    ExplanationDynamicConstraint *explanation) {
  if (provide_input_explanation_) {
    return ExplainLiteralPropagationInput(literal, state, explanation);
  } else {
    return ExplainLiteralPropagationOutput(literal, state, explanation);
  }
}
ExplanationGeneric *WatchedPbSumConstraint::ExplainFailure(
    SolverState &state, ExplanationDynamicConstraint *explanation) {
  if (provide_input_explanation_) {
    return ExplainFailureInput(state, explanation);
  } else {
    return ExplainFailureOutput(state, explanation);
  }
}

WatchedPbSumConstraint::~WatchedPbSumConstraint() {
  if (encoder_ != nullptr)
    delete encoder_;
}
void WatchedPbSumConstraint::UpdateCounts(std::vector<BooleanLiteral> &lits,
                                          SolverState &state) {

  propagate_count_++;

  // TODO if scheduled return
  bool add_partial = encoder_->EncodingPartialAdded(); // TODO scheduled
  if (!add_partial) {
    int size = std::min(max_, (int)lit_sum_);
    add_partial = propagate_count_ > size * encoder_->add_delay;
  }
  if (add_partial) {
    if (!encoder_->EncodingAdded()) {
      state.scheduled_dynamic_constraints_.push_back(this);
    }
  }
}
void WatchedPbSumConstraint::Synchronise(SolverState &state) {
  while(!set_literals_.empty() && (!state.assignments_.IsAssigned(set_literals_.back()))) {
    BooleanLiteral lit = set_literals_.back();
    set_literals_.pop_back();
    int index = lit_to_input_index_[lit];
    WeightedLiteral wl = inputs_[index];
    true_count_ -= wl.weight;
    assert(true_count_ >= 0);
  }
  while(!set_outputs_.empty() && (!state.assignments_.IsAssigned(set_outputs_.back().literal))) {
    WeightedLiteral wl = set_outputs_.back();
    set_outputs_.pop_back();
    if (set_outputs_.empty()) {
      current_max_ = max_;
    } else {
      current_max_ = set_outputs_.back().weight - 1;
    }
    assert(current_max_ >= 0 && current_max_ <= max_);
  }
}
bool WatchedPbSumConstraint::CountCorrect(SolverState &state,
                                          BooleanLiteral literal) {
  return true;
}
} // namespace Pumpkin