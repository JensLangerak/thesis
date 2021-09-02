//
// Created by jens on 18-05-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANBOTTOMLAYERS_EXPLANATION_PB_BOTTOM_LAYERS_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANBOTTOMLAYERS_EXPLANATION_PB_BOTTOM_LAYERS_H_

#include "../../explanation_generic.h"
#include <vector>
namespace Pumpkin {
class SolverState;
class WatchedPbBottomLayersConstraint;
class ExplanationPbBottomLayers: public ExplanationGeneric {
public:
  /// Get the explanation for the conflict
  /// \param constraint
  /// \param state
  ExplanationPbBottomLayers(WatchedPbBottomLayersConstraint * constraint, SolverState &state);
  /// Get the explanation for the propagated value.
  ExplanationPbBottomLayers(WatchedPbBottomLayersConstraint * constraint, SolverState &state, BooleanLiteral propagated_literal);

  BooleanLiteral operator[](int index) const override;
  virtual BooleanLiteral operator[](size_t index) const;
  virtual size_t Size() const;
  ~ExplanationPbBottomLayers() {};

private:
  std::vector<BooleanLiteral> lits_;
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANBOTTOMLAYERS_EXPLANATION_PB_BOTTOM_LAYERS_H_
