//
// Created by jens on 24-05-21.
//

#ifndef PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_DATABASE_PSEUDO_BOOLEAN_2_H_
#define PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_DATABASE_PSEUDO_BOOLEAN_2_H_

#include "../database_dynamic_constraint.h"
#include "../watcher_dynamic_constraint.h"
#include "../watch_list_dynamic.h"
#include <cstdint>
namespace Pumpkin {
class SolverState;
class PseudoBooleanConstraint;
class WatchedPseudoBooleanConstraint2;
class DatabasePseudoBoolean2 {//; : public DatabaseDynamicConstraint {
public:
  explicit DatabasePseudoBoolean2(uint64_t num_vars);

  /// Create a new WatchedCardinalityConstraint from the given constraint. Add the constraint to the watched lists.
  ///
  /// \param constraint
  /// \param state
  /// \return the constructed constraint.
  WatchedPseudoBooleanConstraint2* AddPermanentConstraint(PseudoBooleanConstraint &constraint,
                                                          SolverState &state);

  WatchListDynamic<WatchedPseudoBooleanConstraint2, WatcherDynamicConstraint<WatchedPseudoBooleanConstraint2>> watch_list_true_;
  std::vector<WatchedPseudoBooleanConstraint2*> permanent_constraints_;

  ~DatabasePseudoBoolean2();// override;
private:
  void AddWatchers(WatchedPseudoBooleanConstraint2 *constraint);


};
} // namespace Pumpkin

#endif // PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_DATABASE_PSEUDO_BOOLEAN_2_H_
