//
// Created by jens on 02-06-21.
//

#include "ctt_benchmark.h"
#include "ctt.h"
#include "ctt_converter.h"
#include "parser.h"

namespace simple_sat_solver::ctt {
using namespace simple_sat_solver::benchmark;
Pumpkin::ProblemSpecification CttBenchmark::GetProblem() {
  problem_ = Parser::Parse(problem_file_full_path_);
  CttConverter converter(problem_);
  sat_problem_ = converter.GetSatProblem();
  problem_specification_ = ConvertSatToPumpkin(sat_problem_);
  return problem_specification_;

   }
}