//
// Created by jens on 18-05-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANBOTTOMLAYERS_WATCHER_PB_BOTTOM_LAYERS_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANBOTTOMLAYERS_WATCHER_PB_BOTTOM_LAYERS_CONSTRAINT_H_


#include "watched_pb_bottom_layers_constraint.h"
namespace Pumpkin {
class WatcherPbBottomLayersConstraint {
public:
  WatcherPbBottomLayersConstraint() : constraint_(NULL) {};
  WatcherPbBottomLayersConstraint(WatchedPbBottomLayersConstraint *watched_constraint, int weight)
      : constraint_(watched_constraint), weight_(weight){};

  WatcherPbBottomLayersConstraint& operator=(const WatcherPbBottomLayersConstraint &w) {
    this->constraint_ = w.constraint_;
    this->weight_ = w.weight_;
    return *this;
  }

//private:
  //TODO make private again
  WatchedPbBottomLayersConstraint *constraint_;

  uint32_t weight_; //TODO
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEANBOTTOMLAYERS_WATCHER_PB_BOTTOM_LAYERS_CONSTRAINT_H_
