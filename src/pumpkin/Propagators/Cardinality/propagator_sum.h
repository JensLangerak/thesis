//
// Created by jens on 16-11-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_PROPAGATOR_SUM_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_PROPAGATOR_SUM_H_
#include "../propagator_generic.h"
#include "database_cardinality.h"
namespace Pumpkin {

class PropagatorSum : public PropagatorGeneric {
public:
  explicit PropagatorSum(int64_t num_variables);

  ExplanationGeneric *ExplainLiteralPropagation(BooleanLiteral literal,
                                                SolverState &state)
      override; // returns the conjunction that forces the assignment of input
  // literal to true. Assumes the input literal is not undefined.
  ExplanationGeneric *ExplainFailure(SolverState &state)
      override; // returns the conjunction that leads to failure

  ReasonGeneric *ReasonLiteralPropagation(BooleanLiteral literal,
                                          SolverState &state) override;
  ReasonGeneric *ReasonFailure(SolverState &state) override;

  bool PropagateLiteral(BooleanLiteral true_literal,
      SolverState
          &state) override; // does only a single literal propagation, which is useful since it allows us to then ask simpler propagators to propagate with respect to the new enqueued literal before going further with this propagator

private:
  //TODO DatabaseCardinality *cardinality_database_;
};
}

#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_PROPAGATOR_SUM_H_
