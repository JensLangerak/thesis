//
// Created by jens on 04-09-20.
//

#include "parser.h"

#include <fstream>
#include <iostream>
#include <sstream>

namespace simple_sat_solver {
Solver *DimacsFileParser::Parse(std::string path) {
  std::ifstream satFileStream(path);
  if (!satFileStream.is_open()) {
    std::cout << "Cannot open file: " << path << std::endl;
    satFileStream.close();
    return nullptr;
  }

  s_ = new Solver();
  if (!ReadHeader(satFileStream)) {
    std::cout << "Wrong type or format" << std::endl;
    satFileStream.close();
    delete s_;
    s_ = nullptr;
    return nullptr;
  }

  if (!ReadClauses(satFileStream)) {
    delete s_;
    s_ = nullptr;
  }

  satFileStream.close();
  return s_;
}
bool DimacsFileParser::ReadHeader(std::ifstream &satFileStream) {
  std::string line;

  while (getline(satFileStream, line)) {
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
  sstream >> nrVars_;
  sstream >> nrClauses_;
  for (int i = 0; i < nrVars_; i++)
    s_->NewVar();
  return true;
}
bool DimacsFileParser::ReadClauses(std::ifstream &satFileStream) {
  std::string line;
  while (getline(satFileStream, line)) {
    --nrClauses_;
    Vec<Lit> clause;
    std::stringstream ss;
    ss << line;

    int lit;
    ss >> lit;
    while (lit != 0) {
      Lit l(abs(lit) - 1, lit < 0);
      if (l.x >= nrVars_) {
        std::cout << std::endl << "ID is higher than expected " << std::endl;
        return false;
      }
      clause.push_back(l);
      ss >> lit;
    }

    s_->AddClause(clause);
    if (nrClauses_ == 0)
      break;
  }
  return true;
}
} // namespace simple_sat_solver
