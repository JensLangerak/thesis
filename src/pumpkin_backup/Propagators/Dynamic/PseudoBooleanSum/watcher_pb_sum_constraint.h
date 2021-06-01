//
// Created by jens on 16-03-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANPbSum_WATCH_PB_PbSum_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANPbSum_WATCH_PB_PbSum_CONSTRAINT_H_

#include "watched_pb_sum_constraint.h"
namespace Pumpkin {
class WatcherPbSumConstraint {
public:
  WatcherPbSumConstraint() : constraint_(NULL) {};
  WatcherPbSumConstraint(WatchedPbSumConstraint *watched_constraint, int weight)
  : constraint_(watched_constraint), weight_(weight){};

  WatcherPbSumConstraint& operator=(const WatcherPbSumConstraint &w) {
    this->constraint_ = w.constraint_;
    this->weight_ = w.weight_;
    return *this;
  }

//private:
  //TODO make private again
  WatchedPbSumConstraint *constraint_;

  uint32_t weight_; //TODO
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANPbSum_WATCH_PB_PbSum_CONSTRAINT_H_
