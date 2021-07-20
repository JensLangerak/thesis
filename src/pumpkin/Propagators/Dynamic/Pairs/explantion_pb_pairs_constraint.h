//
// Created by jens on 07-06-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PAIRS_EXPLANTION_PB_PAIRS_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PAIRS_EXPLANTION_PB_PAIRS_CONSTRAINT_H_

#include "../explanation_dynamic_constraint.h"
#include "watched_pb_pairs_constraint.h"
#include "pairs_database.h"
#include <unordered_set>
namespace Pumpkin {
class ExplanationPbPairsConstraint {
public:
  /// Get the explanation for the conflict
  /// \param constraint
  /// \param state
  static void InitExplanationPbPairsConstraint(WatchedPbPairsConstraint * constraint, SolverState &state, ExplanationDynamicConstraint * explanation);
  /// Get the explanation for the propagated value.
  static void InitExplanationPbPairsConstraint(WatchedPbPairsConstraint * constraint, SolverState &state, BooleanLiteral propagated_literal, ExplanationDynamicConstraint * explanation);

  static void InitLits(int64_t size);

  static double time_used_;
  static double avg_pair_size_;
  static int replace_count_;
private:
  static void ReplaceLits(std::vector<WeightedLiteral> causes, SolverState &state, PairsDatabase * pairs_database, std::vector<BooleanLiteral> &lits_);
  static void ReplaceLits2Wrapper(std::vector<WeightedLiteral> causes, WatchedPbPairsConstraint* constraint, SolverState &state, PairsDatabase * pairs_database, std::vector<BooleanLiteral> &lits_);
  static void ReplaceLits2(std::unordered_set<BooleanLiteral> &causes, WatchedPbPairsConstraint * constraint, SolverState &state, PairsDatabase * pairs_database, std::vector<BooleanLiteral> &lits_);

  static std::unordered_set<BooleanLiteral> unreplaced_lits;
  static std::unordered_set<BooleanLiteral> replaced_lits;





};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PAIRS_EXPLANTION_PB_PAIRS_CONSTRAINT_H_
