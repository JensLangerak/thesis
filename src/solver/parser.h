//
// Created by jens on 04-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_SOLVER_PARSER_H_
#define SIMPLESATSOLVER_SRC_SOLVER_PARSER_H_

#include "solver.h"

#include <string>

#include "solver.h"

namespace simple_sat_solver::solver {
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
  /// \param sat_file_stream
  /// \return false when something went wrong.
  bool ReadHeader(std::ifstream &sat_file_stream);

  /// Read and add the clauses to the solver.
  /// \param sat_file_stream
  /// \return false when something went wrong.
  bool ReadClauses(std::ifstream &sat_file_stream);
  int nr_vars_;
  int nr_clauses_;
  Solver *s_; // the solver that is constructed.
};
} // namespace simple_sat_solver::solver
#endif // SIMPLESATSOLVER_SRC_SOLVER_PARSER_H_
