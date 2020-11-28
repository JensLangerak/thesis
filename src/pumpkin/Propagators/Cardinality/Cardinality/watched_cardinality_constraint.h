//
// Created by jens on 16-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_WATCHED_CARDINALITY_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_WATCHED_CARDINALITY_CONSTRAINT_H_

#include "../../../Basic Data Structures/boolean_literal.h"
#include "../Encoders/i_encoder.h"
#include <vector>
namespace Pumpkin {
class ExplanationCardinalityConstraint;
class SolverState;
class WatchedCardinalityConstraint {
public:
  WatchedCardinalityConstraint(std::vector<BooleanLiteral> &literals, int min,
                               int max, IEncoder *encoder);
  ~WatchedCardinalityConstraint() ;
  ExplanationCardinalityConstraint *
  ExplainLiteralPropagation(BooleanLiteral literal, SolverState &state)
      const; // returns the conjunction that forces the assignment of input
             // literal to true. Assumes the input literal is not undefined.
  ExplanationCardinalityConstraint *ExplainFailure(SolverState &state)
      const; // returns the conjunction that leads to failure

  std::vector<BooleanLiteral> literals_;
  int min_;
  int max_;

  int true_count_debug_;
  int true_count_;
  int false_count_;
  int trigger_count_ = 0;
  IEncoder *encoder_;
};
} // namespace Pumpkin
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_WATCHED_CARDINALITY_CONSTRAINT_H_
