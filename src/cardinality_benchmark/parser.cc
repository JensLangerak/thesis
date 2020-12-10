//
// Created by jens on 28-10-20.
//

#include "parser.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include "../sat/constraints/cardinality_constraint.h"

namespace simple_sat_solver::cardinality_benchmark {
using namespace sat;
SatProblem* Parser::Parse(std::string path, int max) {
  SatProblem* problem = new SatProblem(0);
  ReadSat(*problem, path + "/mobsfile");
  ReadCardinality(*problem, path + "/faultfile", max);
  return problem;
}
void Parser::ReadSat(SatProblem &problem, std::string path) {
  std::ifstream file_stream(path);
  if (!file_stream.is_open()) {
    std::cout << "Cannot open file: " << path << std::endl;
    file_stream.close();
    throw "Cannot open file";
  }
  std::string line;
  while (getline(file_stream, line)) {
    std::vector<Lit> clause;
    std::stringstream ss;
    ss << line;

    int lit;
    ss >> lit;
    while (lit != 0) {
      Lit l(abs(lit) - 1, lit < 0);
      if (l.x >= problem.GetNrVars()) {
        int add = l.x - problem.GetNrVars() + 1;
        if (add > 0)
          problem.AddNewVars(add);
        else
          throw "Error";
      }
      clause.push_back(l);
      ss >> lit;
    }
    problem.AddClause(clause);
  }
}
void Parser::ReadCardinality(SatProblem &problem, std::string path, int max) {
  std::ifstream file_stream(path);
  if (!file_stream.is_open()) {
    std::cout << "Cannot open file: " << path << std::endl;
    file_stream.close();
    throw "Cannot open file";
  }
  std::string line;
  std::vector<Lit> clause;
  while (getline(file_stream, line)) {
    std::stringstream ss;
    ss << line;

    int lit;
    ss >> lit;
    Lit l(abs(lit) - 1, lit < 0);
    if (l.x >= problem.GetNrVars()) {
        throw "Error";
    }
//    clause.push_back(l);
    problem.AddToMinimize(l);
  }
//  problem.AddConstraint(new CardinalityConstraint(clause,0,max));

}

}
