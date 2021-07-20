//
// Created by jens on 07-06-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PAIRS_WATCHED_PB_PAIRS_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PAIRS_WATCHED_PB_PAIRS_CONSTRAINT_H_

#include "../../../Utilities/problem_specification.h"
#include "../watched_dynamic_constraint.h"
#include <stack>
#include <unordered_map>
#include "pairs_database.h"
namespace Pumpkin {
class WatchedPbPairsConstraint : public WatchedDynamicConstraint {
public:
  WatchedPbPairsConstraint(PseudoBooleanConstraint &constraint);
  ~WatchedPbPairsConstraint() {
      int test = 2;
  };
  ExplanationGeneric *
  ExplainLiteralPropagation(BooleanLiteral literal, SolverState &state, ExplanationDynamicConstraint * explanation) override; // returns the conjunction that forces the assignment of input
  // literal to true. Assumes the input literal is not undefined.
  ExplanationGeneric *ExplainFailure(SolverState &state, ExplanationDynamicConstraint * explanation) override; // returns the conjunction that leads to failure
  void AddScheduledEncoding(SolverState &state) override;

  std::vector<WeightedLiteral> intput_liters_;
  int max_;
  int current_sum_value_;
  uint32_t lit_sum_ =0 ;

  int max_weight_;
  std::unordered_map<BooleanLiteral, uint32_t> lit_weights_;

  struct DecisionLevelCount {
    int level;
    int sum_value;
    DecisionLevelCount(int level, int sum_value) : level(level), sum_value(sum_value) {}
  };
  std::stack<DecisionLevelCount> decision_level_sums_;
  void UpdateDecisionLevelSum(SolverState &state);
  void Synchronise(SolverState &state);
  bool CountCorrect(SolverState &state, BooleanLiteral literal);



  int propagate_count_ = 0;
  std::unordered_map<BooleanLiteral, int> lit_count_;

  std::vector<BooleanLiteral> add_next_literals_;
  PairsDatabase * pairs_database_;
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PAIRS_WATCHED_PB_PAIRS_CONSTRAINT_H_
