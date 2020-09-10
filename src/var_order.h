//
// Created by jens on 03-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_VAR_ORDER_H_
#define SIMPLESATSOLVER_SRC_VAR_ORDER_H_

#include "types.h"
namespace simple_sat_solver {
class VarOrder {
 public:
  VarOrder();

  void NewVar();
  void Update(Var x);
  void UpdateAll();
  Var Select(const Vec<LBool> &values);

  void Undo(Var i);
 private:
  struct VarActivity {
    VarActivity(Var i, double activity);
    Var var;
    double activity;
    inline bool operator<(const VarActivity & rhs) const {
      return this->activity < rhs.activity;
    }
  };
  Vec<double> activities_;
  Vec<bool> inQueue_;
  std::priority_queue<VarActivity, Vec<VarActivity>> queue_;
  double varDecayFactor;
  double varIncValue;

  void RescaleVars();
};
}
#endif //SIMPLESATSOLVER_SRC_VAR_ORDER_H_
