//
// Created by jens on 18-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_SAT_SAT_PROBLEM_H_
#define SIMPLESATSOLVER_SRC_SAT_SAT_PROBLEM_H_

#include "types.h"
#include <vector>

namespace simple_sat_solver::sat {
// TODO perhaps let the problem be a struct and let this class be a builder
// however this will mean that the sudoku builder uses a sat builder to create
// sat, might not be ideal/overkill.
// TODO allow perhaps arbitrary id types
class SatProblem {
public:
  explicit SatProblem(int nr_vars) : nr_vars_(nr_vars){};
  void AddClause(const std::vector<Lit> &lits);
  inline void AtLeastOne(const std::vector<Lit> &lits) { AddClause(lits); };
  void AtMostOne(const std::vector<Lit> &lits);
  void AtMostK(int k, const std::vector<Lit> &lits);
  void ExactlyOne(const std::vector<Lit> &lits);
  void Implies(const Lit &antecedent, const Lit &consequent );
  inline int GetNrVars() const { return nr_vars_; }
  inline std::vector<std::vector<Lit>> GetClauses() const { return clauses_; };

  bool TestAssignment(const std::vector<bool> &vars) const;

private:
  // TODO move to types
  static bool TestLit(const Lit &l, const std::vector<bool> &vars);
  static bool TestClause(const std::vector<Lit> &c,
                         const std::vector<bool> &vars);

  int nr_vars_;
  std::vector<std::vector<Lit>> clauses_;
};
} // namespace simple_sat_solver::sat

#endif // SIMPLESATSOLVER_SRC_SAT_SAT_PROBLEM_H_
