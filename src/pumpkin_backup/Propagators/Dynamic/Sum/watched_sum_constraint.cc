//
// Created by jens on 16-10-20.
//

#include "watched_sum_constraint.h"
#include "../../../Engine/solver_state.h"
#include "../../../Basic Data Structures/boolean_literal.h"
#include "explanation_sum_constraint.h"
namespace Pumpkin {


WatchedSumConstraint::WatchedSumConstraint(std::vector<BooleanLiteral> &inputs, std::vector<BooleanLiteral> outputs, IEncoder<SumConstraint> *encoder)
    : encoder_(encoder), inputs_(inputs), outputs_(outputs), true_count_(0), false_count_(0){}

ExplanationSumConstraint *
WatchedSumConstraint::ExplainLiteralPropagation(
    BooleanLiteral literal, SolverState &state) const {
  return new ExplanationSumConstraint(this, state, literal);
}
ExplanationSumConstraint *
WatchedSumConstraint::ExplainFailure(SolverState &state) const {
  return new ExplanationSumConstraint(this, state);
}
WatchedSumConstraint::~WatchedSumConstraint() {
  if (encoder_ != nullptr)
    delete encoder_;
}
}
