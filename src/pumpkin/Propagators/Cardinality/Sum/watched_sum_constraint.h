//
// Created by jens on 16-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_SUM_WATCHED_SUM_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_SUM_WATCHED_SUM_CONSTRAINT_H_

#include "../../../Basic Data Structures/boolean_literal.h"
#include <vector>
namespace Pumpkin {
class ExplanationSumConstraint;
class SolverState;
class IEncoder;
class WatchedSumConstraint {
public:
  WatchedSumConstraint(std::vector<BooleanLiteral> &inputs, std::vector<BooleanLiteral> outputs, IEncoder *encoder);
  ~WatchedSumConstraint() ;
  ExplanationSumConstraint *
  ExplainLiteralPropagation(BooleanLiteral literal, SolverState &state)
      const; // returns the conjunction that forces the assignment of input
             // literal to true. Assumes the input literal is not undefined.
  ExplanationSumConstraint *ExplainFailure(SolverState &state)
      const; // returns the conjunction that leads to failure

  std::vector<BooleanLiteral> inputs_;
  std::vector<BooleanLiteral> outputs_;

  std::vector<BooleanLiteral> true_order_;

  int true_count_debug_;
  int true_count_;
  int false_count_;
  int trigger_count_ = 0;
  IEncoder *encoder_;
};
} // namespace Pumpkin
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_WATCHED_CARDINALITY_CONSTRAINT_H_
