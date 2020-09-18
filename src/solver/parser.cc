//
// Created by jens on 04-09-20.
//

#include "parser.h"

#include <fstream>
#include <iostream>
#include <sstream>

namespace simple_sat_solver::solver {
Solver *DimacsFileParser::Parse(std::string path) {
  std::ifstream sat_file_stream(path);
  if (!sat_file_stream.is_open()) {
    std::cout << "Cannot open file: " << path << std::endl;
    sat_file_stream.close();
    return nullptr;
  }

  s_ = new Solver();
  if (!ReadHeader(sat_file_stream)) {
    std::cout << "Wrong type or format" << std::endl;
    sat_file_stream.close();
    delete s_;
    s_ = nullptr;
    return nullptr;
  }

  if (!ReadClauses(sat_file_stream)) {
    delete s_;
    s_ = nullptr;
  }

  sat_file_stream.close();
  return s_;
}
bool DimacsFileParser::ReadHeader(std::ifstream &sat_file_stream) {
  std::string line;

  while (getline(sat_file_stream, line)) {
    if (line[0] != 'c')
      break;
  }

  std::string type = "p cnf ";
  if (line.rfind(type, 0) != 0) {
    return false;
  }

  line.erase(0, type.length());
  std::stringstream sstream;
  sstream << line;
  sstream >> nr_vars_;
  sstream >> nr_clauses_;
  for (int i = 0; i < nr_vars_; i++)
    s_->NewVar();
  return true;
}
bool DimacsFileParser::ReadClauses(std::ifstream &sat_file_stream) {
  std::string line;
  while (getline(sat_file_stream, line)) {
    --nr_clauses_;
    Vec<Lit> clause;
    std::stringstream ss;
    ss << line;

    int lit;
    ss >> lit;
    while (lit != 0) {
      Lit l(abs(lit) - 1, lit < 0);
      if (l.x >= nr_vars_) {
        std::cout << std::endl << "ID is higher than expected " << std::endl;
        return false;
      }
      clause.push_back(l);
      ss >> lit;
    }

    s_->AddClause(clause);
    if (nr_clauses_ == 0)
      break;
  }
  return true;
}
} // namespace simple_sat_solver::solver
