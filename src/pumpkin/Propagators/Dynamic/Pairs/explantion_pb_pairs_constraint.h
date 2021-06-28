//
// Created by jens on 07-06-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PAIRS_EXPLANTION_PB_PAIRS_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PAIRS_EXPLANTION_PB_PAIRS_CONSTRAINT_H_

#include "../explanation_dynamic_constraint.h"
#include "watched_pb_pairs_constraint.h"
#include "pairs_database.h"
namespace Pumpkin {
class ExplanationPbPairsConstraint {
public:
  /// Get the explanation for the conflict
  /// \param constraint
  /// \param state
  static void InitExplanationPbPairsConstraint(WatchedPbPairsConstraint * constraint, SolverState &state, ExplanationDynamicConstraint * explanation);
  /// Get the explanation for the propagated value.
  static void InitExplanationPbPairsConstraint(WatchedPbPairsConstraint * constraint, SolverState &state, BooleanLiteral propagated_literal, ExplanationDynamicConstraint * explanation);

private:
  static void ReplaceLits(std::vector<WeightedLiteral> causes, SolverState &state, PairsDatabase * pairs_database, std::vector<BooleanLiteral> &lits_);
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PAIRS_EXPLANTION_PB_PAIRS_CONSTRAINT_H_
