//
// Created by jens on 10-12-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_WATCHER_PSEUDO_BOOLEAN_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_WATCHER_PSEUDO_BOOLEAN_CONSTRAINT_H_

#include "watched_pseudo_boolean_constraint.h"

namespace Pumpkin {
class WatcherPseudoBooleanConstraint2 {
public:
  WatcherPseudoBooleanConstraint2() : constraint_(nullptr){};
  WatcherPseudoBooleanConstraint2(
      WatchedPseudoBooleanConstraint2 *watched_constraint, int weight)
      : constraint_(watched_constraint), weight_(weight){};

  WatcherPseudoBooleanConstraint2 &
  operator=(const WatcherPseudoBooleanConstraint2 &w) {
    this->constraint_ = w.constraint_;
    this->weight_ = w.weight_;
    return *this;
  }

  // private:
  // TODO make private again
  WatchedPseudoBooleanConstraint2 *constraint_;
  int weight_;
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_WATCHER_PSEUDO_BOOLEAN_CONSTRAINT_H_
