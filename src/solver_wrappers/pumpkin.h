//
// Created by jens on 15-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_SOLVER_WRAPPERS_PUMPKIN_H_
#define SIMPLESATSOLVER_SRC_SOLVER_WRAPPERS_PUMPKIN_H_

#include "i_solver.h"
namespace Pumpkin {
class ProblemSpecification;
}
namespace simple_sat_solver::solver_wrappers {
class Pumpkin : public ISolver {
public:
  enum class CardinalityOption{ Totolizer, Sequential, Propagator};
  inline Pumpkin(CardinalityOption cardinality_option, bool add_encodings) : add_encodings_(add_encodings), solved_(false), cardinality_option_(cardinality_option){};
  bool Solve(const sat::SatProblem &p) override;
  bool Optimize(const sat::SatProblem &p) override;
  ::Pumpkin::ProblemSpecification ConvertProblem(sat::SatProblem &p);
  std::vector<bool> GetSolution() const override;
  ~Pumpkin() {}

private:
  std::vector<bool> solution_;
  bool solved_;
  bool add_encodings_;
  CardinalityOption cardinality_option_;

};
}

#endif // SIMPLESATSOLVER_SRC_SOLVER_WRAPPERS_PUMPKIN_H_
