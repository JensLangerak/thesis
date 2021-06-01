//
// Created by jens on 17-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_SUM_WATCHER_SUM_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_SUM_WATCHER_SUM_CONSTRAINT_H_

#include "watched_sum_constraint.h"
namespace Pumpkin {
class WatcherSumConstraint {
public:
  WatcherSumConstraint() : constraint_(NULL) {};
  WatcherSumConstraint(WatchedSumConstraint *watched_constraint)
      : constraint_(watched_constraint){};

  WatcherSumConstraint& operator=(const WatcherSumConstraint &w) {
    this->constraint_ = w.constraint_;
    return *this;
  }

//private:
  //TODO make private again
  WatchedSumConstraint *constraint_;
};

} // namespace Pumpkin
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_WATCHER_CARDINALITY_CONSTRAINT_H_
