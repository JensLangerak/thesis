//
// Created by jens on 03-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_VAR_ORDER_H_
#define SIMPLESATSOLVER_SRC_VAR_ORDER_H_

#include "types.h"
namespace simple_sat_solver {
class VarOrder {
 public:
  VarOrder(Vec<LBool> &ref_to_assigns, Vec<double> &ref_to_activity);

  void NewVar();
  void Update(Var x);
  void UpdateAll();
  void Undo(Var x);
  Var Select();

};
}
#endif //SIMPLESATSOLVER_SRC_VAR_ORDER_H_
