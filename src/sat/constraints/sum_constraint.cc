//
// Created by jens on 20-11-20.
//

#include "sum_constraint.h"
namespace simple_sat_solver::sat {

IConstraint *SumConstraint::Clone() { return new SumConstraint(*this); }
}
