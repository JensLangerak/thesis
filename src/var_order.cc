//
// Created by jens on 03-09-20.
//

#include "var_order.h"
namespace simple_sat_solver {
// TODO create needed datastructure
void VarOrder::NewVar() {
  Var v = queue_.size();
  activities_.push_back(1.0);
  queue_.push(VarActivity(v, 1.0));
  inQueue_.push_back(true);
  varIncValue = 1.0;
  varDecayFactor = 0.95;

}
void VarOrder::Update(Var x) {
  activities_[x] += varIncValue;
  if (activities_[x] > 1e100) {
    RescaleVars();
  }
  queue_.push(VarActivity(x, activities_[x]));
  inQueue_[x] = true;

}
void VarOrder::RescaleVars() {
  while(!queue_.empty())
    queue_.pop();
  for (int i = 0; i < activities_.size(); i++) {
    activities_[i] *= 1e-100;
    if (inQueue_[i])
      queue_.push(VarActivity(i, activities_[i]));
  }
  varIncValue *= 1e-100;
}
void VarOrder::UpdateAll() {
  varIncValue *= varDecayFactor;
}
Var VarOrder::Select(const Vec<LBool> &values) {
  while (!queue_.empty()) {
    Var v = queue_.top().var;
    queue_.pop();
    inQueue_[v] = false;
    if (values[v] == LBool::kUnknown)
      return v;
  }
  for (LBool v : values) {
    if (v == LBool::kUnknown)
      throw "Error";
  }
  return -1;
}
void VarOrder::Undo(Var x) {
  if (!inQueue_[x]) {
    queue_.push(VarActivity(x, activities_[x]));
    inQueue_[x] = true;
  }
}
VarOrder::VarOrder() {

}
VarOrder::VarActivity::VarActivity(Var i, double activity):var(i), activity(activity) {
}
}