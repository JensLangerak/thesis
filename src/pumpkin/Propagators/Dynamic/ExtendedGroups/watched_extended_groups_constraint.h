//
// Created by jens on 10-08-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_EXTENDEDGROUPS_WATCHED_EXTENDED_GROUPS_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_EXTENDEDGROUPS_WATCHED_EXTENDED_GROUPS_CONSTRAINT_H_

#define USEFAILURE
#define ADDASPERM
#include "../../../Utilities/problem_specification.h"
#include "../watched_dynamic_constraint.h"
#include <stack>
#include <unordered_map>
#include <unordered_set>
namespace Pumpkin {
class WatchedExtendedGroupsConstraint : public WatchedDynamicConstraint {
public:
  WatchedExtendedGroupsConstraint(PseudoBooleanConstraint &constraint);
  ~WatchedExtendedGroupsConstraint() {
    int test = 2;
  };
  ExplanationGeneric *
  ExplainLiteralPropagation(BooleanLiteral literal, SolverState &state, ExplanationDynamicConstraint * explanation) override; // returns the conjunction that forces the assignment of input
  // literal to true. Assumes the input literal is not undefined.
  ExplanationGeneric *ExplainFailure(SolverState &state, ExplanationDynamicConstraint * explanation) override; // returns the conjunction that leads to failure
  void AddScheduledEncoding(SolverState &state) override;

  std::vector<WeightedLiteral> intput_liters_;
  std::vector<BooleanLiteral> set_literals_;
  std::vector<BooleanLiteral> auxiliary_literals_;
  std::vector<int> auxiliray_boundary_;
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

  bool add_aux_literal_;
  void AddAuxiliaryLiteral(SolverState &state);

#ifdef USEFAILURE
  std::unordered_set<BooleanLiteral> false_literals_group_;
  std::unordered_set<BooleanLiteral> not_false_literals_group_;
  BooleanLiteral false_literal_ = BooleanLiteral();
  int false_literal_weight_ = 0;
#endif

  int reuse_count = 0;
  int total_count = 0;
  bool create_new_failure_ = true;

  int wrong_level = 0;
  int tot = 0;
  int used = 0;
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_EXTENDEDGROUPS_WATCHED_EXTENDED_GROUPS_CONSTRAINT_H_
