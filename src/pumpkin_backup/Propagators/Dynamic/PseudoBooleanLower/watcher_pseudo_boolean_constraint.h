//
// Created by jens on 10-12-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANLOWER_WATCHER_PSEUDO_BOOLEAN_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANLOWER_WATCHER_PSEUDO_BOOLEAN_CONSTRAINT_H_

#include "watched_pseudo_boolean_constraint.h"

namespace Pumpkin {
class WatcherPseudoBooleanConstraint3 {
public:
  WatcherPseudoBooleanConstraint3() : constraint_(nullptr){};
  WatcherPseudoBooleanConstraint3(
      WatchedPseudoBooleanConstraint3 *watched_constraint, int weight)
      : constraint_(watched_constraint), weight_(weight){};

  WatcherPseudoBooleanConstraint3 &
  operator=(const WatcherPseudoBooleanConstraint3 &w) {
    this->constraint_ = w.constraint_;
    this->weight_ = w.weight_;
    return *this;
  }

  // private:
  // TODO make private again
  WatchedPseudoBooleanConstraint3 *constraint_;
  int weight_;
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_WATCHER_PSEUDO_BOOLEAN_CONSTRAINT_H_
