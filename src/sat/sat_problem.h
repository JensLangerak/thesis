//
// Created by jens on 18-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_SAT_SAT_PROBLEM_H_
#define SIMPLESATSOLVER_SRC_SAT_SAT_PROBLEM_H_

#include "types.h"
#include <vector>
#include "constraints/i_constraint.h"

namespace simple_sat_solver::sat {
// TODO allow perhaps arbitrary id types

/// Class that describes a SAT problem. Clauses can be added directly with the
/// AddClause method. When adding constraints, the corresponding clauses will be
/// generated and added.
class SatProblem {
public:
  /// Create a new sat problem with nr_vars vars. Note that certain constraints
  /// create helper vars and therefore increase the final amount of vars.
  /// \param nr_vars the number of vars int the sat problem.
  explicit SatProblem(int nr_vars) : nr_vars_(nr_vars < 0 ? 0 : nr_vars){};
  SatProblem() : SatProblem(0) {};

  /// Add the clause to the sat problem.
  /// \param lits the clause
  void AddClause(const std::vector<Lit> &lits);

  /// Add a constraint that at least one of the given lits must be true.
  /// \param lits
  inline void AtLeastOne(const std::vector<Lit> &lits) { AddClause(lits); };

  /// Add a constraint that at most one of the given lits may be true.
  /// \param lits
  void AtMostOne(const std::vector<Lit> &lits);

  /// Add a constraint that at most k of the given lits may be true.
  /// For k=1 it is recommended to use AtLeastOne.
  /// \param k the max number of lits that is allowed to be true.
  /// \param lits
  void AtMostK(int k, const std::vector<Lit> &lits);
//  void AddCardinalityConstraint(const std::vector<Lit> &lits, int min, int max);
  void AddConstraint(IConstraint* constraint);

  /// Add a constraint the exactly one of the given lits must be true.
  /// \param lits
  void ExactlyOne(const std::vector<Lit> &lits);

  /// Add a implication constraint. If the antecedent is true, then the
  /// consequent should also be true. If the antecedent is false, the consequent
  /// can have any value.
  /// \param antecedent
  /// \param consequent
  void Implies(const Lit &antecedent, const Lit &consequent);

  /// Add a constraint that all the given lits must be false.
  /// \param lits
  void None(const std::vector<Lit> &lits);

  /// Return the number of vars.
  inline int GetNrVars() const { return nr_vars_; }

  /// Return a list containing all the clauses.
  inline std::vector<std::vector<Lit>> GetClauses() const { return clauses_; };

  /// Test if a assignment satisfies the problem.
  /// \param vars assignment to be tested.
  /// \return true if the assignment satisfies the problem.
  bool TestAssignment(const std::vector<bool> &vars) const;

  int AddNewVar();

  int AddNewVars(int nr_vars);

  std::vector<IConstraint*> GetConstraints() const;

  inline void AddToMinimize(Lit l) {minimize_.push_back(WeightedLit(l,1));};
  inline void AddToMinimize(WeightedLit l) {minimize_.push_back(l);};

  std::vector<WeightedLit> GetMinimizeLit();
  ~SatProblem();

  SatProblem(const SatProblem & problem);

private:
  // TODO move to types
  static bool TestLit(const Lit &l, const std::vector<bool> &vars);
  static bool TestClause(const std::vector<Lit> &c,
                         const std::vector<bool> &vars);

  int nr_vars_;
  std::vector<std::vector<Lit>> clauses_;
  std::vector<IConstraint*> constraints;
  std::vector<WeightedLit> minimize_; //TODO different optimize functions
};
} // namespace simple_sat_solver::sat

#endif // SIMPLESATSOLVER_SRC_SAT_SAT_PROBLEM_H_
