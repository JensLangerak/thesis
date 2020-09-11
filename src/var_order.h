//
// Created by jens on 03-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_VAR_ORDER_H_
#define SIMPLESATSOLVER_SRC_VAR_ORDER_H_

#include "types.h"
namespace simple_sat_solver {
/// Class keeps a relative order of all the vars based on their recent activity.
class VarOrder {
 public:
  VarOrder();

  /// Create the activity values for the new var.
  void NewVar();

  /// Increase the activity score of x.
  /// \param x
  void Update(Var x);

  /// Decrease all the activity scores.
  void UpdateAll();

  /// Return the most active unknown value.
  /// \param values values of the vars.
  /// \return the most active unknown value, -1 if there are no unkown values.
  Var Select(const Vec<LBool> &values);

  /// Handle i getting unknown again. Make sure it is in the queue.
  /// \param i a var that has become unknown.
  void Undo(Var i);
 private:
  struct VarActivity {
    Var var;
    double activity;
    VarActivity(Var i, double activity) : var(i), activity(activity) {};

    inline bool operator<(const VarActivity & rhs) const {
      return this->activity < rhs.activity;
    }
  };

  /// Rescale all the activity scores.
  void RescaleVars();

  Vec<double> activities_;
  Vec<bool> inQueue_; // Keeps track which var are up to date in the queue.
  std::priority_queue<VarActivity, Vec<VarActivity>> queue_;
  double varDecayFactor;
  double varIncValue;

};
}
#endif //SIMPLESATSOLVER_SRC_VAR_ORDER_H_
