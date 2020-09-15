//
// Created by jens on 03-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_CLAUSE_H_
#define SIMPLESATSOLVER_SRC_CLAUSE_H_

#include "types.h"
namespace simple_sat_solver::solver {
class Solver;
class Clause {
public:
  /// Creates a new clause and add watchers to the first two literals.
  /// Duplicated literals are removed.
  ///
  /// \param lits literals for the clause.
  /// \param learnt true if this is a learnt clause.
  /// \param s solver whose literals should be watched.
  Clause(Vec<Lit> lits, bool learnt, Solver &s);

  /// Creates a new unit clause. Set the watchers to the unitLit and
  /// mostRecentLit.
  /// It does not set the unit literal to true, nor will it add
  /// anything to the propagation list. Thus after creating such class the
  /// literal must be set to true. It assumes that there are no duplicates in
  /// lits. If the clause has only 1 literal, mostRecentLearnt will be ignored.
  ///
  /// \param lits literals for the clause.
  /// \param learnt true if this is a learnt clause.
  /// \param s solve whose literals should be watched.
  /// \param unitLit the literal that currently can be used for unit
  /// propagation.
  /// \param mostRecentLearnt the most recent learnt literal in
  /// lits. This is needed to handle undo's.
  Clause(const Vec<Lit> &lits, bool learnt, Solver &s, Lit unitLit,
         Lit mostRecentLearnt);

  ~Clause();

  /// Locks the clause. It will not be deleted when the clause db is reduced.
  void Lock();
  /// Unlocks the clause. If it is a learnt clause it can be deleted by de
  /// clause db.
  void Unlock();
  /// Returns True if the clause is locked.
  /// \Return true if the clause is locked.
  bool Locked() const;
  /// Removes the clause from the watch lists in s.
  /// \param s solver that contains the watchers.
  void Remove(Solver &s);

  /// Simplifies the clauses. If it is unit try to set the var to true.
  /// Should be called at the start of each solve attempt to set fill the
  /// propagation queue with the unit vars. Returns false if is is not possible
  /// to make the clause true with the current partial assignment.
  /// \param S the solver that contains the clause.
  /// \return false if it is not possible to make the clause true.
  bool Simplify(Solver &S);

  /// Propagates p over this clause and update the clause activity.
  /// It adds the clause to a watcher list and checks if the clause has become
  /// unit or false. Clause activity in increased when the clause has become
  /// unit or false.
  /// The other watched literal only evaluates to false when p was the only
  /// literal that did not evaluate to false.
  ///
  /// If p evaluates to true, re-add the clause to its watcher list.
  /// If p evaluates to false:
  ///     1) If the other watched literal evaluates to true, re-add the clause
  /// to p's watcher list.
  ///     2) If now all the clauses are false, re-add the clause to the watcher
  /// lists of p and return false.
  ///     3) If there is a literal with an unknown value that is not watched,
  /// start watching that literal
  ///     4) This means that only the other watched literal is not false. Set
  /// that one to true and re-add the clause to p's watcher list.
  ///
  /// On returning the clause will always have added itself to a watcher list.
  /// The clause should always watch two different literals, unless it has only
  /// 1 literal.
  ///
  /// \param S solver that contains the clause.
  /// \param p is a literal in the clause and is watched by this clause.
  /// \return false if all literals evaluate to true. Otherwise return true.
  bool Propagate(Solver &S, Lit p);

  /// Returns all literals except the literals over the same var as p.
  /// Should be called on the clause that set p to true. Thus its other literals
  /// are false.
  ///
  /// \param p the literal that was set to true by this clause.
  /// \return the literals in this clause except p.
  Vec<Lit> CalcReason(Lit p) const;
  /// Return all the literals.
  /// Should be called on the clause that is false.
  /// \return the literals in the clause.
  Vec<Lit> CalcReason() const;

  /// Prints the constraint.
  void PrintConstraint() const;

  /// Prints the constraint and replaces the variables with there value.
  /// \param vars values for the variables.
  void PrintFilledConstraint(const Vec<LBool> &vars) const;

  /// Activity score is multiplied by 1e-100;
  void RescaleActivity();

  /// Test the clause and return true if the constraint can be resolved.
  /// \param s solver that is used to get the values.
  /// \return true if one of the clauses literal is true.
  bool Value(const Solver &s) const;

  /// Debug function to check if the state of the watchers are correct.
  /// \param s
  void CheckWatchers(const Solver *s) const;

  /// \return the activity score of the clause.
  double GetActivity() const;

private:
  /// Constructor that sets learnt_, lock_ and activity_.
  /// \param learnt
  explicit Clause(bool learnt);

  Vec<Lit> lits_;
  int watchA_;
  int watchB_;
  bool learnt_;
  bool lock_;
  double activity_;
};
} // namespace simple_sat_solver::solver

#endif // SIMPLESATSOLVER_SRC_CLAUSE_H_
