//
// Created by jens on 21-06-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_SUM_PB_SUM_HELPER_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_SUM_PB_SUM_HELPER_H_

#include "../Encoders/generalized_totaliser_sum_nodes.h"

namespace Pumpkin {
class SolverState;
class PropagatorPbSumOutput;
class PropagatorPbSumInput;

class PbSumHelper {
public:
  static WatchedPbSumConstraint *
  AddConstraint(PbSumConstraint constraint, SolverState &state,
                GeneralizedTotaliserSumNodes::Factory *p_nodes);

//protected:
  static PropagatorPbSumInput * input_propagator_;
  static PropagatorPbSumOutput * output_propagator_;
  static void AddPropagators(SolverState &state);

};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_SUM_PB_SUM_HELPER_H_
