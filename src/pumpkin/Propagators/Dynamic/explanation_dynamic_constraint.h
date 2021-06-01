//
// Created by jens on 20-05-21.
//

#ifndef PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_EXPLANATION_DYNAMIC_CONSTRAINT_H_
#define PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_EXPLANATION_DYNAMIC_CONSTRAINT_H_

#include "../explanation_generic.h"
#include <vector>
namespace Pumpkin {
class ExplanationDynamicConstraint : public ExplanationGeneric {
public:
//  /// Get the explanation for the conflict
//  /// \param constraint
//  /// \param state
//  ExplanationDynamicConstraint(WatchedPseudoBooleanConstraint2 * constraint, SolverState &state);
//  /// Get the explanation for the propagated value.
//  ExplanationDynamicConstraint(WatchedPseudoBooleanConstraint2 * constraint, SolverState &state, BooleanLiteral propagated_literal);

  BooleanLiteral operator[](int index) const override;
  virtual BooleanLiteral operator[](size_t index) const;
  virtual size_t Size() const;

  ExplanationDynamicConstraint(std::vector<BooleanLiteral> lits) : lits_(lits) {};

  ExplanationDynamicConstraint();
  void Init(std::vector<BooleanLiteral> lits)  { lits_ = lits;};
protected:
  std::vector<BooleanLiteral> lits_;
};
} // namespace Pumpkin

#endif // PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_EXPLANATION_DYNAMIC_CONSTRAINT_H_
