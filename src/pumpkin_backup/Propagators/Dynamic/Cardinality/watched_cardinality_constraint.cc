//
// Created by jens on 16-10-20.
//

#include "watched_cardinality_constraint.h"
#include "../../../../logger/logger.h"
#include "../../../Engine/solver_state.h"
#include "explanation_cardinality_constraint.h"
namespace Pumpkin {

int WatchedCardinalityConstraint::next_log_id_ = 0;
WatchedCardinalityConstraint::WatchedCardinalityConstraint(
    std::vector<BooleanLiteral> &literals, int min, int max, IEncoder<CardinalityConstraint> * encoder) : encoder_(encoder), literals_(literals), min_(min), max_(max), true_count_(0), false_count_(0){
  log_id_ = next_log_id_;
  ++next_log_id_;
  encoder_->log_id_ = log_id_;
  simple_sat_solver::logger::Logger::Log2("Constraint: " + std::to_string(log_id_) + " - " + std::to_string(max) + " out of " + std::to_string(literals.size()));
  for (BooleanLiteral l : literals)
    lit_count_[l.ToPositiveInteger()] = 0;
}

ExplanationCardinalityConstraint *
WatchedCardinalityConstraint::ExplainLiteralPropagation(
    BooleanLiteral literal, SolverState &state) {
  return new ExplanationCardinalityConstraint(this, state, literal);
}
ExplanationCardinalityConstraint *
WatchedCardinalityConstraint::ExplainFailure(SolverState &state) {
  return new ExplanationCardinalityConstraint(this, state);
}
WatchedCardinalityConstraint::~WatchedCardinalityConstraint() {
  if (encoder_ != nullptr)
    delete encoder_;
}
void WatchedCardinalityConstraint::UpdateCounts(
    std::vector<BooleanLiteral> &lits, SolverState &state) {
  propagate_count++;
  for (BooleanLiteral l : lits) {
    lit_count_[l.ToPositiveInteger()]++;
  }
  if (!encoder_->AddOnRestart())
    return;
  if (propagate_count > literals_.size() * max_ * encoder_->add_delay) {
    if (!encoder_->EncodingAdded()) {
      state.propagator_cardinality_.add_constraints_.push(this);
      if (encoder_->EncodingPartialAdded()) {
        for (BooleanLiteral l : lits) {
          if (lit_count_[l.ToPositiveInteger()] > max_ * encoder_->add_delay)
            add_next_literals_.push_back(l);
        }
      } else {
        for (BooleanLiteral l : literals_) {
          if (lit_count_[l.ToPositiveInteger()] > max_ * encoder_->add_delay) {
            add_next_literals_.push_back(l);
          }
        }
      }
    }
  }
}
}
