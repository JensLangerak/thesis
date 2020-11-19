//
// Created by jens on 28-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_CARDINALITY_BENCHMARK_PARSER_H_
#define SIMPLESATSOLVER_SRC_CARDINALITY_BENCHMARK_PARSER_H_

#include <string>
#include "../sat/sat_problem.h"
namespace simple_sat_solver::cardinality_benchmark {
class Parser {
public:
  Parser() {};
  sat::SatProblem *Parse(std::string path, int max);

private:
  void ReadSat(sat::SatProblem &problem, std::string path);
  void ReadCardinality(sat::SatProblem &problem, std::string path, int max);
};
}

#endif // SIMPLESATSOLVER_SRC_CARDINALITY_BENCHMARK_PARSER_H_
