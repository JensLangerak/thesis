//
// Created by jens on 19-11-20.
//

#include "cardinality_constraint.h"
namespace simple_sat_solver::sat {

CardinalityConstraint::~CardinalityConstraint() {


}
IConstraint *CardinalityConstraint::Clone() { return new CardinalityConstraint(*this);}
}
