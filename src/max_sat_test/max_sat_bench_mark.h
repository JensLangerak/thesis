//
// Created by jens on 01-06-21.
//

#ifndef SIMPLESATSOLVER_SRC_MAX_SAT_TEST_MAX_SAT_BENCH_MARK_H_
#define SIMPLESATSOLVER_SRC_MAX_SAT_TEST_MAX_SAT_BENCH_MARK_H_

#include "../benchmark/benchmark.h"
namespace simple_sat_solver::max_sat {
using namespace benchmark;
class MaxSatBenchMark : public Benchmark {
protected:
  Pumpkin::ProblemSpecification GetProblem() override;
  void ClusterWeights(Pumpkin::ProblemSpecification & problems, int clusters);
};
}

#endif // SIMPLESATSOLVER_SRC_MAX_SAT_TEST_MAX_SAT_BENCH_MARK_H_
