//
// Created by jens on 25-01-21.
//

#ifndef SIMPLESATSOLVER_SRC_PSEUDO_BOOLEAN_OPB_PARSER_H_
#define SIMPLESATSOLVER_SRC_PSEUDO_BOOLEAN_OPB_PARSER_H_
#include "../sat/constraints/pseudo_boolean_constraint.h"
#include "../sat/sat_problem.h"
#include <map>
#include <string>
namespace simple_sat_solver::pseudo_boolean {
class OpbParser {
public:
  static sat::SatProblem Parse(std::string path);

private:
  OpbParser(std::ifstream *fstream) : fstream(fstream), problem(0){};

  void Parse();
  std::ifstream *fstream;
  sat::SatProblem problem;
  std::vector<sat::WeightedLit> read_min_line(std::stringstream &stringstream);
  sat::WeightedLit getWeightedLit(int w, std::string basic_string);

  std::map<std::string, sat::Lit> string_lit_map;
  void read_constraint(std::stringstream &stringstream);
};
}
#endif // SIMPLESATSOLVER_SRC_PSEUDO_BOOLEAN_OPB_PARSER_H_
