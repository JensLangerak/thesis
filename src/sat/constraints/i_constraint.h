//
// Created by jens on 19-11-20.
//

#ifndef SIMPLESATSOLVER_SRC_SAT_CONSTRAINTS_I_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_SAT_CONSTRAINTS_I_CONSTRAINT_H_

namespace simple_sat_solver::sat {
  class IConstraint {
  public:
    virtual ~IConstraint();
    virtual IConstraint* Clone() =0;
  protected:
    IConstraint()  {}
  };

}
#endif // SIMPLESATSOLVER_SRC_SAT_CONSTRAINTS_I_CONSTRAINT_H_
