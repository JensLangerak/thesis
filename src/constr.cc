//
// Created by jens on 03-09-20.
//

#include "constr.h"
namespace simple_sat_solver {
void Constr::Remove(simple_sat_solver::Solver *s) {

}
bool Constr::Propagate(Solver *s, Lit p) {
  return false;
}
bool Constr::Simplify(Solver *s) {
  return false;
}
void Constr::Undo(Solver *s, Lit p) {

}
void Constr::CalcReason(Solver *s, Lit p, Vec<Lit> out_reason) {

}
Constr::~Constr() {

}
const void Constr::PrintConstraint() {

}
const void Constr::PrintFilledConstraint(const Vec<LBool> &vars) {

}
}
