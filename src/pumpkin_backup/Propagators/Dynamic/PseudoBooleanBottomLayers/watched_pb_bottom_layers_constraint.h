//
// Created by jens on 18-05-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANBOTTOMLAYERS_WATCHED_PB_BOTTOM_LAYERS_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANBOTTOMLAYERS_WATCHED_PB_BOTTOM_LAYERS_CONSTRAINT_H_


#include "../../../Basic Data Structures/boolean_literal.h"
#include "../../../Basic Data Structures/problem_specification.h"
#include "../Encoders/i_encoder.h"
#include "sum_node.h"
#include <map>
#include <unordered_map>
#include <vector>
namespace Pumpkin {
class SolverState;
class ExplanationPbBottomLayers;
class WatchedPbBottomLayersConstraint {
public:
  WatchedPbBottomLayersConstraint(std::vector<BooleanLiteral> &inputs,
                                  std::vector<uint32_t> &input_weights,
                                  int max);
  ~WatchedPbBottomLayersConstraint();
  void EncodeBottomLayers(SolverState & state, int nr_levels);
  ExplanationPbBottomLayers *
  ExplainLiteralPropagation(BooleanLiteral literal, SolverState &state);
  // returns the conjunction that forces the assignment of input
  // literal to true. AsPbSumes the input literal is not undefined.
  ExplanationPbBottomLayers *ExplainFailure(SolverState &state);
  // returns the conjunction that leads to failure

  std::vector<WeightedLiteral> inputs_;
  std::unordered_map<int, int> lit_to_input_index_;

  std::unordered_map<int, int> weight_output_index_map_;

  //  std::unordered_map<int, int> weight_cause_index_;
  //  std::vector<int> weight_trail_;
  //  std::vector<int> weight_trail_delimitors_;
  std::vector<BooleanLiteral> set_literals_;

  int current_max_;

  int true_count_debug_;
  int true_count_;
  int trigger_count_ = 0;
  IEncoder<PbSumConstraint> *encoder_ = nullptr;

  void UpdateCounts(std::vector<BooleanLiteral> &lits, SolverState &state);
  int propagate_count = 0;
  uint32_t lit_sum_ = 0;
  int max_ = 0;

  uint32_t max_weight_;
  int debug_index;
  static int next_debug_index;
  int max_cause_weight;

  std::vector<SumNode> nodes_;
  std::map<int, int> lit_to_node_;
  void update_sum_(BooleanLiteral literal, uint32_t weight);
  int current_sum_value_ = 0;
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANBOTTOMLAYERS_WATCHED_PB_BOTTOM_LAYERS_CONSTRAINT_H_
