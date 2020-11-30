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
    std::vector<BooleanLiteral> &literals, int min, int max, IEncoder * encoder) : encoder_(encoder), literals_(literals), min_(min), max_(max), true_count_(0), false_count_(0){
  log_id_ = next_log_id_;
  ++next_log_id_;
  encoder_->log_id_ = log_id_;
  simple_sat_solver::logger::Logger::Log2("Constraint: " + std::to_string(log_id_) + " - " + std::to_string(max) + " out of " + std::to_string(literals.size()));
}

ExplanationCardinalityConstraint *
WatchedCardinalityConstraint::ExplainLiteralPropagation(
    BooleanLiteral literal, SolverState &state) const {
  return new ExplanationCardinalityConstraint(this, state, literal);
}
ExplanationCardinalityConstraint *
WatchedCardinalityConstraint::ExplainFailure(SolverState &state) const {
  return new ExplanationCardinalityConstraint(this, state);
}
WatchedCardinalityConstraint::~WatchedCardinalityConstraint() {
  if (encoder_ != nullptr)
    delete encoder_;
}
}
