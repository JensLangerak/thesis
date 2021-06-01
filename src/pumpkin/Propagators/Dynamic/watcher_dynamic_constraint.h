//
// Created by jens on 20-05-21.
//

#ifndef PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_WATCHER_DYNAMIC_CONSTRAINT_H_
#define PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_WATCHER_DYNAMIC_CONSTRAINT_H_

namespace Pumpkin {
template <class Watched>
class WatcherDynamicConstraint {
public:
  WatcherDynamicConstraint() : constraint_(nullptr){};
  WatcherDynamicConstraint(
      Watched *watched_constraint, int weight)
  : constraint_(watched_constraint), weight_(weight){};

  WatcherDynamicConstraint &
  operator=(const WatcherDynamicConstraint &w) {
    this->constraint_ = w.constraint_;
    this->weight_ = w.weight_;
    return *this;
  }

  virtual ~WatcherDynamicConstraint() {
      int tesrt = 2;
  };

  Watched *constraint_;
  int weight_; //TODO perhaps template for additional info

};
} // namespace Pumpkin

#endif // PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_WATCHER_DYNAMIC_CONSTRAINT_H_
