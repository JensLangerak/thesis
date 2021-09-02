//
// Created by jens on 18-06-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_SUM_WATCHED_PB_SUM_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_SUM_WATCHED_PB_SUM_CONSTRAINT_H_

#include "../../../../pumpkin/Utilities/boolean_literal.h"
#include "../../../../pumpkin/Utilities/problem_specification.h"
#include "../watched_dynamic_constraint.h"
#include <unordered_map>
#include <vector>
namespace Pumpkin {
class ExplanationPbSumConstraintInput;
class ExplanationPbSumConstraintOutput;
class SolverState;
class WatchedPbSumConstraint : public WatchedDynamicConstraint {
public:
  WatchedPbSumConstraint(PbSumConstraint constraint,IEncoder<PbSumConstraint>::IFactory *encoder_factory);
  ~WatchedPbSumConstraint() override;
  ExplanationGeneric *
  ExplainLiteralPropagationInput(BooleanLiteral literal, SolverState &state, ExplanationDynamicConstraint * explanation); // returns the conjunction that forces the assignment of input
  // literal to true. Assumes the input literal is not undefined.
  ExplanationGeneric *ExplainFailureInput(SolverState &state, ExplanationDynamicConstraint * explanation); // returns the conjunction that leads to failure

  ExplanationGeneric *
  ExplainLiteralPropagationOutput(BooleanLiteral literal, SolverState &state, ExplanationDynamicConstraint * explanation); // returns the conjunction that forces the assignment of input
  // literal to true. Assumes the input literal is not undefined.
  ExplanationGeneric *ExplainFailureOutput(SolverState &state, ExplanationDynamicConstraint * explanation); // returns the conjunction that leads to failure

  void UpdateCounts(std::vector<BooleanLiteral> &lits, SolverState &state);
  void AddScheduledEncoding(SolverState &state) override;

  bool provide_input_explanation_ = false; // TODO better solution
  ExplanationGeneric *
  ExplainLiteralPropagation(BooleanLiteral literal, SolverState &state,
                            ExplanationDynamicConstraint *explanation) override;
  ExplanationGeneric *
  ExplainFailure(SolverState &state,
                 ExplanationDynamicConstraint *explanation) override;
  IEncoder<PbSumConstraint> *encoder_;
//private:
  std::vector<WeightedLiteral> inputs_;
  std::unordered_map<BooleanLiteral, int> lit_to_input_index_;
  std::vector<WeightedLiteral> outputs_;
  std::unordered_map<BooleanLiteral, int> lit_to_output_index_;

  std::unordered_map<int,int> weight_output_index_map_;

  std::vector<BooleanLiteral> set_literals_;
  std::vector<WeightedLiteral> set_outputs_;

  int current_max_;

  int trigger_count_ = 0;
  int true_count_ = 0;

  int propagate_count_ =0;
  uint32_t lit_sum_=0;
  int max_ = 0;

  uint32_t max_weight_ = 0;
  int max_cause_weight_;

  void Synchronise(SolverState &state);
  bool CountCorrect(SolverState &state, BooleanLiteral literal);
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_SUM_WATCHED_PB_SUM_CONSTRAINT_H_
