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
Constr::~Constr() {

}
const void Constr::PrintConstraint() {

}
const void Constr::PrintFilledConstraint(const Vec<LBool> &vars) {

}
Vec<Lit> Constr::CalcReason(Lit p) {
  return simple_sat_solver::Vec<Lit>();
}
Vec<Lit> Constr::CalcReason() {
  return simple_sat_solver::Vec<Lit>();
}
void Constr::UndoUnitWatch(Solver *s) {

}
}
