//
// Created by jens on 02-06-21.
//

#ifndef SIMPLESATSOLVER_SRC_CTT_CTT_BENCHMARK_H_
#define SIMPLESATSOLVER_SRC_CTT_CTT_BENCHMARK_H_
#include "../benchmark/benchmark.h"
#include "../sat/sat_problem.h"
#include "ctt.h"
namespace simple_sat_solver::ctt {
using namespace simple_sat_solver::benchmark;
using namespace simple_sat_solver::sat;
class CttBenchmark : public Benchmark {
protected:
  Pumpkin::ProblemSpecification GetProblem() override;
  Ctt problem_;
  SatProblem *sat_problem_;
  Pumpkin::ProblemSpecification problem_specification_;
};
}
#endif // SIMPLESATSOLVER_SRC_CTT_CTT_BENCHMARK_H_
