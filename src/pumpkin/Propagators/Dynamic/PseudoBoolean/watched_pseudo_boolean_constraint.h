//
// Created by jens on 10-12-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_WATCHED_PSEUDO_BOOLEAN_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_WATCHED_PSEUDO_BOOLEAN_CONSTRAINT_H_

#include "../../../Basic Data Structures/boolean_literal.h"
#include "../Encoders/i_encoder.h"
#include "../../../Basic Data Structures/problem_specification.h"
#include <unordered_map>
namespace Pumpkin {
class ExplanationPseudoBooleanConstraint2;
class SolverState;
class WatchedPseudoBooleanConstraint2 {
public:
  WatchedPseudoBooleanConstraint2(std::vector<BooleanLiteral> &literals, std::vector<uint32_t> &weights, int max, IEncoder<PseudoBooleanConstraint> *encoder);
  ~WatchedPseudoBooleanConstraint2();
  ExplanationPseudoBooleanConstraint2 *
  ExplainLiteralPropagation(BooleanLiteral literal, SolverState &state); // returns the conjunction that forces the assignment of input
  // literal to true. Assumes the input literal is not undefined.
  ExplanationPseudoBooleanConstraint2 *ExplainFailure(SolverState &state); // returns the conjunction that leads to failure

//  struct WeightedLiteral {
//    WeightedLiteral(BooleanLiteral l, int w) : lit(l), weight(w) {};
//    BooleanLiteral lit;
//    int weight;
//  };

  std::vector<WeightedLiteral> literals_;
  int max_;

  int current_sum_value;
  int trigger_count_ = 0;
  IEncoder<PseudoBooleanConstraint> *encoder_;
  int log_id_;
  static int next_log_id_;
  std::unordered_map<int, int> lit_count_;
  int propagate_count =0;
  std::vector<BooleanLiteral> add_next_literals_;
  int true_count_debug_ = 0;
  void UpdateCounts(std::vector<BooleanLiteral> &lits, SolverState &state);
};
} // namespace Pumpkin
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_WATCHED_PSEUDO_BOOLEAN_CONSTRAINT_H_
