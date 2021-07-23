//
// Created by jens on 24-05-21.
//

#ifndef PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_WATCHED_PSEUDO_BOOLEAN_CONSTRAINT_2_H_
#define PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_WATCHED_PSEUDO_BOOLEAN_CONSTRAINT_2_H_

#include "../watched_dynamic_constraint.h"
#include "../Encoders/i_encoder.h"
#include <stack>
#include <unordered_map>
namespace Pumpkin {
struct WeightedLiteral;
class SolverState;
class PseudoBooleanConstraint;
class WatchedPseudoBooleanConstraint2 : public WatchedDynamicConstraint {
public:
  WatchedPseudoBooleanConstraint2(PseudoBooleanConstraint &constraint, IEncoder<PseudoBooleanConstraint>::IFactory *encoding_factory);
  ~WatchedPseudoBooleanConstraint2();
  ExplanationGeneric *
  ExplainLiteralPropagation(BooleanLiteral literal, SolverState &state, ExplanationDynamicConstraint * explanation); // returns the conjunction that forces the assignment of input
  // literal to true. Assumes the input literal is not undefined.
  ExplanationGeneric *ExplainFailure(SolverState &state, ExplanationDynamicConstraint * explanation); // returns the conjunction that leads to failure
  void UpdateCounts(std::vector<BooleanLiteral> &lits, SolverState &state);
  void AddScheduledEncoding(SolverState &state) override;

  std::vector<WeightedLiteral> current_literals_;
  std::vector<WeightedLiteral> original_literals_;
  std::vector<WeightedLiteral> unencoded_constraint_literals_;
  int max_;
  int current_sum_value_;
  uint32_t lit_sum_ =0 ;
  IEncoder<PseudoBooleanConstraint> *encoder_;

  int max_weight_;
  std::unordered_map<int, uint32_t> lit_weights_;
  std::unordered_map<BooleanVariable, int> lit_usages_;
  void UpdateLitCount(BooleanLiteral lit);
  int GetLitCount(BooleanLiteral lit);

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

  std::vector<BooleanLiteral> add_next_literals_;

  void Reset(SolverState &state);
};
} // namespace Pumpkin

#endif // PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_WATCHED_PSEUDO_BOOLEAN_CONSTRAINT_2_H_
