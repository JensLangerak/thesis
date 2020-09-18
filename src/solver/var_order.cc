//
// Created by jens on 03-09-20.
//

#include "var_order.h"
namespace simple_sat_solver::solver {
// TODO create needed datastructure
void VarOrder::NewVar() {
  Var v = queue_.size();
  activities_.push_back(1.0);
  queue_.push(VarActivity(v, 1.0));
  in_queue_.push_back(true);
}
void VarOrder::Update(Var x) {
  activities_[x] += var_inc_value_;
  // since we fake decreasing all the scores, the value can become very large,
  // only then update everything.
  if (activities_[x] > 1e100) {
    RescaleVars();
  }
  queue_.push(VarActivity(x, activities_[x]));
  in_queue_[x] = true;
}
void VarOrder::RescaleVars() {
  while (!queue_.empty())
    queue_.pop();
  for (int i = 0; i < activities_.size(); i++) {
    activities_[i] *= 1e-100;
    if (in_queue_[i])
      queue_.push(VarActivity(i, activities_[i]));
  }
  var_inc_value_ *= 1e-100;
}

void VarOrder::UpdateAll() {
  // has the same effect as actually decreasing all the values.
  var_inc_value_ *= var_decay_factor_;
}
Var VarOrder::Select(const Vec<LBool> &values) {
  while (!queue_.empty()) {
    Var v = queue_.top().var;
    queue_.pop();
    in_queue_[v] = false;
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
  if (!in_queue_[x]) {
    queue_.push(VarActivity(x, activities_[x]));
    in_queue_[x] = true;
  }
}
VarOrder::VarOrder() : var_decay_factor_(1.0), var_inc_value_(1.0 / 0.95) {}
} // namespace simple_sat_solver::solver