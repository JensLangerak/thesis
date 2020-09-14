//
// Created by jens on 03-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_SOLVER_H_
#define SIMPLESATSOLVER_SRC_SOLVER_H_

#include <stack>

#include "clause.h"
#include "types.h"
#include "var_order.h"

namespace simple_sat_solver {
class Solver {
public:
  Solver();
  ~Solver();

  /// Creates a new var and add to the solver.
  /// \return the newly created var.
  Var NewVar();

  /// Add a new clause to the solver.
  /// \param literals the literals that form the disjunction.
  /// \return true if the clause could be added.
  bool AddClause(const Vec<Lit> &literals);

  /// Returns true if the problem is satisfiable.
  /// Returns false if the problem is unsatisfiable.
  /// If it returns true, a satisfiable model can retrieved with GetModel()
  /// \return true if the problem is satisfiable.
  bool Solve();

  /// Returns a the solution found by Solve().
  /// \return the solution found by Solve()
  Vec<bool> GetModel() const;

  /// Print the problem.
  void PrintProblem();
  /// Print the current variable assignments.
  void PrintAssignments();
  /// Print the problem and fill in the assigned values.
  void PrintFilledProblem();

  /// Try to set the lit to true and update the solver state.
  /// If lit was already set to true, nothing changes.
  /// If lit was already set to false, return false.
  /// If lit is not set, set it to true. Add ~lit to the propagation queue,
  /// update the history and register that constr is the reason for the lit
  /// value.
  /// \param lit lit that must be set to true.
  /// \param constr clause that causes the lit to become true. Can be null, if
  /// it is a decision/assumption.
  /// \return false if the lit was already set to false.
  bool SetLitTrue(Lit lit, Clause *constr);

  /// Get the current value of l.
  /// \param l
  /// \return the value of l.
  LBool GetLitValue(Lit l) const;

  /// Add clause to the watcher list of lit.
  /// The propagation function of clause is called when lit becomes false.
  /// \param lit literal to be watched.
  /// \param clause the clause that watches lit.
  void AddWatch(Lit &lit, Clause *clause);

  /// Remove clause from the watcher list of lit.
  /// \param lit
  /// \param p_clause
  void RemoveFromWatchList(Lit &lit, const Clause *clause);

  /// Get the index of lit in the watcher list.
  /// \param lit
  /// \return the index of lit in the watcher list.
  static int LitIndex(Lit &lit);

  /// Rescale the clause activities.
  /// Multiply all by 1e-100
  void RescaleClauseActivity();

  // TODO move
  double constrIncActivity_;
  double constrDecayFactor_;
  Queue<Lit> propagationQueue_;
  Vec<Vec<Clause *>> watches_;

private:
  /// Propagates the information on the queue to the clauses that watch the
  /// literals on the queue.
  /// If one of these literals makes a clause false, a conflict is found. The
  /// conflicting clause is returned via the conflict parameter and false is
  /// returned.
  /// When it returns true, the propagation queue is empty, with false
  /// this is not the case.
  /// \param conflict output parameter that holds a pointer to the clause that
  /// causes the conflict (if any).
  /// \return false if a conflict is found.
  bool Propagate(Clause *&conflict);

  /// Creates a new decision level and starts the new decision level by setting
  /// lit to true.
  /// \param lit the literal that must be set to true.
  void Assume(Lit lit);

  /// True if all vars have a value.
  /// \return true if all values are set to true of false.
  bool AllAssigned() const;

  /// Undo all the moves that are made with a level equal or higher to level.
  /// \param level the lowest level that must be undone.
  void UndoDecisions(int level);

  /// Analyze the conflict. Return the literals that cause the conflict.
  /// If all the returned literals are false then the conflict will occur.
  /// Undoes some of the moves done since the last decision that led to this
  /// conflict. However the decision might not be undone.
  /// \param clause that is false.
  /// \return the literals that cause the clause to become false. These literals
  /// are false as well.
  Vec<Lit> Analyze(const Clause *constr);

  /// Backtrack to the given decision level. Undoes all the moves up to and
  /// including the last decision with a level higher than the desired level.
  /// Thus on return the decision that is than considered to be last made has a
  /// level lower or equal to the given level. Its propagated values are not
  /// undone, thus it is possible that the most recent updated var has a level
  /// higher than the given level.
  /// \param level the level to which the solver must backtrack.
  /// \return true if it succeeded.
  bool Backtrack(int level);

  /// Undo one var assignment. Return false if there are no assignments to be
  /// undone.
  /// \return true if it could undo a assignment.
  bool UndoOne();

  /// Learn a new clause that can be used to prevent the conflict. Backtrack
  /// to the oldest decision where the new clause is unit.
  /// If it is not possible to backtrack far enough the problem is
  /// unsatisfiable.
  /// \param conflict clause that causes the conflict, thus all its lits are
  /// false.
  /// \return false if it could not backtrack far enough.
  bool HandleConflict(const Clause * conflict);

  /// Make a new decision. Var is selected by VarOrder. If there are no var
  /// without value unknown return false.
  /// \return true if it could decide on a var.
  bool AddAssumption();

  /// Try to solve the sat problem using CDCL. Without exceeding the limits
  /// given by its parameters.
  ///     Returns true if it has found a solution. Solution is kept and can be
  /// obtained with GetModel.
  ///     Returns false if it has derived (or detected) a clause that will
  /// always be false. The solver can no longer be used.
  ///     Returns unknown if it exceeded the limits given by its parameters.
  /// It this case it keeps a part of its learned clauses and backtracks to the
  /// base level. This method can be called again (with different parameters),
  /// which may lead to a solution.
  /// \param maxLearnt max number of clauses that can be learned. If it exceeds
  ///        this number, it will forget half of the leaned clauses and returns
  ///        unknown.
  /// \param maxConflicts max number of conflicts. If to many conflicts occur
  ///        unknown is returned.
  /// \return True if it has found a solution, False if it has found a clause
  ///         that will always be false, otherwise unknown.
  LBool Solve(int maxLearnt, int maxConflicts);

  /// Delete roughly half of the learnt clauses. Keep the clauses that are used
  /// to assign values to the current set vars.
  /// \param learnt
  void ReduceDB(int learnt);

  /// Check if the solution is correct. Throws an exception if there is a clause
  /// that is not true, otherwise returns true.
  /// \return true if all the constraints are true.
  LBool CheckConstraints();

  /// Select the lit from the lits that is most recently updated.
  /// \param lits a list of lits which currently have an assigned value.
  /// \return the lit that got its values assigned the latest.
  Lit GetMostRecentLit(Vec<Lit> lits);

  /// Debug function that checks if watchers are in a correct state.
  /// Throws an exception if it is not the case.
  void CheckWatchers();

  VarOrder varOrder; // Used to decide what the most promising decision var is.
  Vec<Clause *> constraints_;   // Original constraints of the problem.
  Vec<Clause *> learntClauses_; // Learnt constraints, can be deleted.
  Vec<LBool> varAssignments_;   // Current assignment of the vars.
  Vec<int> level_; // Stores for each var the decision level that causes its
                   // value. This are the levels in decisionLevel_, not the size
                   // of learnt_. -1 if the values is still unknown.
  Vec<Clause *> reason_; // Keeps track of the clause that causes the value for
                         // var. Null if unknown of decision var.

  std::stack<Lit> learnt_; // Order in which that var got assigned a value.
                           // Oldest on the bottom, newest on the top.
  std::stack<int>
      decisionLevels_; // Stores when the decision vars are added. After a
                       // decision is added to learnt_, learnt_.size() is stored
                       // on decisionLevels.
};
} // namespace simple_sat_solver

#endif // SIMPLESATSOLVER_SRC_SOLVER_H_
