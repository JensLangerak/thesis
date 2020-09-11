//
// Created by jens on 04-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_PARSER_H_
#define SIMPLESATSOLVER_SRC_PARSER_H_

#include "solver.h"

#include <string>

#include "solver.h"

namespace simple_sat_solver {
class DimacsFileParser {
public:
  DimacsFileParser() = default;
  /// Read a file in the DIMACS format and create a solver.
  /// \param path location of the DIMACS file.
  /// \return a solver for the given problem.
  Solver *Parse(std::string path);

private:
  /// Read the header, set the number of clauses and vars.
  /// Add the vars to the solver.
  /// \param satFileStream
  /// \return false when something went wrong.
  bool ReadHeader(std::ifstream &satFileStream);

  /// Read and add the clauses to the solver.
  /// \param satFileStream
  /// \return false when something went wrong.
  bool ReadClauses(std::ifstream &satFileStream);
  int nrVars_;
  int nrClauses_;
  Solver *s_; // the solver that is constructed.
};
} // namespace simple_sat_solver
#endif // SIMPLESATSOLVER_SRC_PARSER_H_
