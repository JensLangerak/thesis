//
// Created by jens on 16-10-20.
//

#include "watched_cardinality_constraint.h"
#include "explanation_cardinality_constraint.h"
#include "../../Engine/solver_state.h"
namespace Pumpkin {

WatchedCardinalityConstraint::WatchedCardinalityConstraint(
    std::vector<BooleanLiteral> &literals, int min, int max, IEncoder * encoder) : encoder_(encoder), literals_(literals), min_(min), max_(max), true_count_(0), false_count_(0){}

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
