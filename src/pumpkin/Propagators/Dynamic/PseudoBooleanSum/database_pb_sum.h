//
// Created by jens on 12-03-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANSUM_DATABASE_PB_SUM_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANSUM_DATABASE_PB_SUM_H_
#include "watch_list_pb_sum.h"
#include "watched_pb_sum_constraint.h"
#include "../../../Basic Data Structures/problem_specification.h"
#include <stdint-gcc.h>
#include <vector>
namespace Pumpkin {
class DatabasePbSum {

  public:
    explicit DatabasePbSum(uint64_t num_vars);

    /// Create a new WatchedSumConstraint from the given constraint. Add the constraint to the watched lists.
    ///
    /// \param constraint
    /// \param state
    /// \return the constructed constraint.
    WatchedPbSumConstraint* AddPermanentConstraint(PbSumConstraint &constraint,
                                                 SolverState &state);

  WatchedPbSumConstraint* AddPermanentConstraint(std::vector<BooleanLiteral> input_lits, std::vector<uint32_t > input_weights, std::vector<BooleanLiteral> output_lits, std::vector<uint32_t> outpu_weights, IEncoder<PbSumConstraint> * encoder, SolverState &state);
  WatchListPbSum watch_list_true;
    std::vector<WatchedPbSumConstraint*> permanent_constraints_;

    ~DatabasePbSum();
  private:
    void AddWatchers(WatchedPbSumConstraint *constraint);
  };
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANSUM_DATABASE_PB_SUM_H_
