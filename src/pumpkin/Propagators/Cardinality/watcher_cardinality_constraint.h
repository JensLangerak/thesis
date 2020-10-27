//
// Created by jens on 17-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_WATCHER_CARDINALITY_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_WATCHER_CARDINALITY_CONSTRAINT_H_

#include "watched_cardinality_constraint.h"
namespace Pumpkin {
class WatcherCardinalityConstraint {
public:
  WatcherCardinalityConstraint() : constraint_(NULL) {};
  WatcherCardinalityConstraint(WatchedCardinalityConstraint *watched_constraint)
      : constraint_(watched_constraint){};

  WatcherCardinalityConstraint& operator=(const WatcherCardinalityConstraint &w) {
    this->constraint_ = w.constraint_;
    return *this;
  }

//private:
  //TODO make private again
  WatchedCardinalityConstraint *constraint_;
};

} // namespace Pumpkin
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_WATCHER_CARDINALITY_CONSTRAINT_H_
