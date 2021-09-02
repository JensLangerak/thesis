//
// Created by jens on 06-07-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_BOTTOMLAYERS_WATCHED_BOTTOM_LAYERS_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_BOTTOMLAYERS_WATCHED_BOTTOM_LAYERS_CONSTRAINT_H_

#include "../watched_dynamic_constraint.h"
#include "../Encoders/i_encoder.h"
#include "sum_node.h"
#include <stack>
#include <unordered_map>
namespace Pumpkin {
struct WeightedLiteral;
class SolverState;
class PseudoBooleanConstraint;
class WatchedBottomLayersConstraint : public WatchedDynamicConstraint {
public:
  explicit WatchedBottomLayersConstraint(PseudoBooleanConstraint &constraint);
  ~WatchedBottomLayersConstraint() = default;
  ExplanationGeneric *
  ExplainLiteralPropagation(BooleanLiteral literal, SolverState &state,
                            ExplanationDynamicConstraint
                                *explanation) override; // returns the conjunction that forces the assignment of input
  // literal to true. Assumes the input literal is not undefined.
  ExplanationGeneric *ExplainFailure(
      SolverState &state,
      ExplanationDynamicConstraint
          *explanation) override; // returns the conjunction that leads to failure

  void EncodeBottomLayers(SolverState & state, int nr_levels);
  std::vector<WeightedLiteral> inputs_;
  std::unordered_map<BooleanLiteral, int> lit_to_input_index_;
  std::unordered_map<int, int> weight_output_index_map_;
  int current_max_;

  int true_count_debug_;
//  int true_count_;
  int trigger_count_ = 0;

  uint32_t lit_sum_ = 0;
  int max_ = 0;
  uint32_t max_weight_;
  int max_cause_weight;

  std::vector<SumNode> nodes_;

  std::stack<int> updated_nodes_index_;
  std::stack<std::pair<int,int>> update_nodes_decision_boundaries_;

  std::unordered_map<BooleanLiteral, int> lit_to_node_index_;
  void UpdateSum(BooleanLiteral literal, uint32_t weight, SolverState & state);
  int current_sum_value_ = 0;
  void UpdateDecisionLevelSum(SolverState & state);

  struct DecisionLevelCount {
    int level;
    int sum_value;
    DecisionLevelCount(int level, int sum_value) : level(level), sum_value(sum_value) {}
  };
  std::stack<DecisionLevelCount> decision_level_sums_;
  void Synchronise(SolverState &state);
  bool CountCorrect(SolverState &state, BooleanLiteral literal);
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_BOTTOMLAYERS_WATCHED_BOTTOM_LAYERS_CONSTRAINT_H_
