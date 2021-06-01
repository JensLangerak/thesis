//
// Created by jens on 16-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_WATCHED_CARDINALITY_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_WATCHED_CARDINALITY_CONSTRAINT_H_

#include "../../../Basic Data Structures/boolean_literal.h"
#include "../../../Basic Data Structures/problem_specification.h"
#include "../Encoders/i_encoder.h"
#include <unordered_map>
#include <vector>
namespace Pumpkin {
class ExplanationCardinalityConstraint;
class SolverState;
class WatchedCardinalityConstraint {
public:
  WatchedCardinalityConstraint(std::vector<BooleanLiteral> &literals, int min,
                               int max, IEncoder<CardinalityConstraint> *encoder);
  ~WatchedCardinalityConstraint() ;
  ExplanationCardinalityConstraint *
  ExplainLiteralPropagation(BooleanLiteral literal, SolverState &state)
      ; // returns the conjunction that forces the assignment of input
             // literal to true. Assumes the input literal is not undefined.
  ExplanationCardinalityConstraint *ExplainFailure(SolverState &state)
      ; // returns the conjunction that leads to failure

  std::vector<BooleanLiteral> literals_;
  int min_;
  int max_;

  int true_count_debug_;
  int true_count_;
  int false_count_;
  int trigger_count_ = 0;
  IEncoder<CardinalityConstraint> *encoder_;
  int log_id_;
  static int next_log_id_;
  std::unordered_map<int, int> lit_count_;
  int propagate_count =0;
  std::vector<BooleanLiteral> add_next_literals_;
  void UpdateCounts(std::vector<BooleanLiteral> &lits, SolverState &state);
};
} // namespace Pumpkin
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_WATCHED_CARDINALITY_CONSTRAINT_H_
