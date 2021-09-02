//
// Created by jens on 21-05-21.
//

#ifndef PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_WATCHED_DYNAMIC_CONSTRAINT_H_
#define PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_WATCHED_DYNAMIC_CONSTRAINT_H_

#include "../../Utilities/boolean_literal.h"
#include "../explanation_generic.h"
#include "explanation_dynamic_constraint.h"

namespace Pumpkin {
class SolverState;
class WatchedDynamicConstraint {
public:
  virtual ExplanationGeneric * ExplainLiteralPropagation(BooleanLiteral literal, SolverState& state, ExplanationDynamicConstraint * explanation) = 0;
  virtual ExplanationGeneric * ExplainFailure(SolverState & state, ExplanationDynamicConstraint * explanation) = 0;

  virtual ~WatchedDynamicConstraint() = default;

  int log_id_;
  static int next_log_id_;
  virtual void AddScheduledEncoding(SolverState &state) {};

protected:
  WatchedDynamicConstraint() = default;
};
} // namespace Pumpkin

#endif // PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_WATCHED_DYNAMIC_CONSTRAINT_H_
