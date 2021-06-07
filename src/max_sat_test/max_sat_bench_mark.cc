//
// Created by jens on 01-06-21.
//

#include "max_sat_bench_mark.h"
namespace simple_sat_solver::max_sat {
Pumpkin::ProblemSpecification MaxSatBenchMark::GetProblem() {
  Pumpkin::ProblemSpecification problem =Pumpkin::ProblemSpecification::ReadMaxSATFormula(problem_file_full_path_);
  for (int i = 0 ; i < problem.weighted_literals_.size(); ++i)
    problem.weighted_literals_[i].weight = 1;
  return problem;
}
}
