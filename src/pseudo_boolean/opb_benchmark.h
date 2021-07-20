//
// Created by jens on 07-07-21.
//

#ifndef SIMPLESATSOLVER_SRC_PSEUDO_BOOLEAN_OPB_BENCHMARK_H_
#define SIMPLESATSOLVER_SRC_PSEUDO_BOOLEAN_OPB_BENCHMARK_H_


#include "../benchmark/benchmark.h"
#include "../sat/sat_problem.h"
#include "opb_parser.h"
namespace simple_sat_solver::pseudo_boolean {
using namespace simple_sat_solver::benchmark;
using namespace simple_sat_solver::sat;
class OpbBenchmark : public Benchmark {
protected:
  Pumpkin::ProblemSpecification GetProblem() override;
  SatProblem * sat_problem_;
  Pumpkin::ProblemSpecification problem_specification_;
};
}

#endif // SIMPLESATSOLVER_SRC_PSEUDO_BOOLEAN_OPB_BENCHMARK_H_
