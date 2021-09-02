//
// Created by jens on 18-05-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANBOTTOMLAYERS_REASON_PB_BOTTOM_LAYERS_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANBOTTOMLAYERS_REASON_PB_BOTTOM_LAYERS_H_

#include "../../reason_generic.h"
#include "explanation_pb_bottom_layers.h"
namespace Pumpkin {
class WatchedPbBottomLayersConstraint;
class ReasonPbBottomLayers : public ReasonGeneric {
public:
  ReasonPbBottomLayers(WatchedPbBottomLayersConstraint *constraint,
                                 SolverState &state);

  ReasonPbBottomLayers(WatchedPbBottomLayersConstraint *constraint,
                                 BooleanLiteral propagated_literal,
                                 SolverState &state);

  void RoundToOne(BooleanLiteral resolving_literal, SolverState &state) override;
  Term operator[](size_t index) override;
  uint64_t GetCoefficient(BooleanLiteral literal) override;
  void MultiplyByFraction(uint64_t numerator, uint64_t denominator) override;
  uint32_t GetRightHandSide() override;
  size_t Size() override;

private:
  ExplanationPbBottomLayers *explanation_;
  uint32_t scaling_factor_;
};
} // namespace Pumpkin

#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANBOTTOMLAYERS_REASON_PB_BOTTOM_LAYERS_H_
