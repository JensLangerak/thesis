//
// Created by jens on 10-12-20.
//

#include "watched_pseudo_boolean_constraint.h"
#include "explanation_pseudo_boolean_constraint.h"
#include "../../../../logger/logger.h"
#include "../../../Engine/solver_state.h"

namespace Pumpkin {

int WatchedPseudoBooleanConstraint2::next_log_id_ = 0;

bool CompareByWeight(const WeightedLiteral &a, WeightedLiteral &b)
{
  return a.weight > b.weight;
}

WatchedPseudoBooleanConstraint2::WatchedPseudoBooleanConstraint2(std::vector<BooleanLiteral> &literals, std::vector<uint32_t> &weights, int max, IEncoder<PseudoBooleanConstraint> *encoder) :
encoder_(encoder), max_(max), current_sum_value(0){
  assert(literals.size() == weights.size());
  max_weight_ = 0;
  for (int i = 0; i < literals.size(); ++i) {
    literals_.push_back(WeightedLiteral(literals[i], weights[i]));
    lit_weights_[literals[i].ToPositiveInteger()] = weights[i];
    if (weights[i] > max_weight_)
      max_weight_ = weights[i];
    lit_sum_ += weights[i];
  }
  std::sort(literals_.begin(), literals_.end(), CompareByWeight);
  log_id_ = next_log_id_;
  ++next_log_id_;
  encoder_->log_id_ = log_id_;
  simple_sat_solver::logger::Logger::Log2("Constraint: " + std::to_string(log_id_) + " - " + std::to_string(max) + " out of " + std::to_string(literals.size()));
  for (BooleanLiteral l : literals)
    lit_count_[l.ToPositiveInteger()] = 0;
}

ExplanationPseudoBooleanConstraint2 *
WatchedPseudoBooleanConstraint2::ExplainLiteralPropagation(
    BooleanLiteral literal, SolverState &state) {
  return new ExplanationPseudoBooleanConstraint2(this, state, literal);
}
ExplanationPseudoBooleanConstraint2 *
WatchedPseudoBooleanConstraint2::ExplainFailure(SolverState &state) {
  return new ExplanationPseudoBooleanConstraint2(this, state);
}
WatchedPseudoBooleanConstraint2::~WatchedPseudoBooleanConstraint2() {
  if (encoder_ != nullptr)
    delete encoder_;
}
void WatchedPseudoBooleanConstraint2::UpdateCounts(
    std::vector<BooleanLiteral> &lits, SolverState &state) {
  propagate_count++;
  for (BooleanLiteral l : lits) {
    lit_count_[l.ToPositiveInteger()]++;
  }
  if (!encoder_->AddOnRestart())
    return;
  bool add_partial = encoder_->EncodingPartialAdded(); // TODO scheduled
  if (!add_partial && encoder_->SupportsIncremental()) {
    int sum = 0;
    for (auto wl : literals_) {
      BooleanLiteral l = wl.literal;
      if (lit_count_[l.ToPositiveInteger()] * wl.weight > max_ * encoder_->add_delay) {
        sum+= wl.weight;
      }
    }
    add_partial = sum > max_ - max_weight_;
  } else if (!add_partial && !encoder_->SupportsIncremental()) {
    add_partial = propagate_count > lit_sum_ * max_ * encoder_->add_delay;
  }
  if (add_partial) {
    if (!encoder_->EncodingAdded()) {
      state.propagator_pseudo_boolean_2_.add_constraints_.push(this);
      if (encoder_->EncodingPartialAdded()) {
        for (BooleanLiteral l : lits) {
          if (lit_count_[l.ToPositiveInteger()]*lit_weights_[l.ToPositiveInteger()] > max_ * encoder_->add_delay)
            add_next_literals_.push_back(l);
        }
      } else {
        for (auto wl : literals_) {
          BooleanLiteral l = wl.literal;
          if (lit_count_[l.ToPositiveInteger()]*lit_weights_[l.ToPositiveInteger()] > max_ * encoder_->add_delay) {
            add_next_literals_.push_back(l);
          }
        }
      }
    }
  }
}
}