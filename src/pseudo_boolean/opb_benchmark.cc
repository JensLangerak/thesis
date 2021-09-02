//
// Created by jens on 07-07-21.
//

#include "opb_benchmark.h"
namespace simple_sat_solver::pseudo_boolean {

Pumpkin::ProblemSpecification OpbBenchmark::GetProblem() {
  sat_problem_ = simple_sat_solver::pseudo_boolean::OpbParser::Parse(problem_file_full_path_);

  problem_specification_ = ConvertSatToPumpkin(sat_problem_);
  return problem_specification_;
}
}