//
// Created by jens on 15-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_SOLVER_WRAPPERS_PUMPKIN_H_
#define SIMPLESATSOLVER_SRC_SOLVER_WRAPPERS_PUMPKIN_H_

#include "i_solver.h"
#include "../pumpkin/Propagators/Cardinality/Encoders/i_encoder.h"
namespace Pumpkin {
class ProblemSpecification;
}
namespace simple_sat_solver::solver_wrappers {
class Pumpkin : public ISolver {
public:
  enum class CardinalityOption{ Totolizer, Sequential, Propagator};
  inline Pumpkin(::Pumpkin::IEncoder::IFactory * encoder_factory, int start_upperbound) : encoder_factory_(encoder_factory), solved_(false), start_uppberboud_(start_upperbound) {};

  bool Solve(const sat::SatProblem &p) override;
  bool Optimize(const sat::SatProblem &p) override;
  ::Pumpkin::ProblemSpecification ConvertProblem(sat::SatProblem &p);
  std::vector<bool> GetSolution() const override;
  ~Pumpkin() override {
    delete encoder_factory_;
  }

  int start_uppberboud_ = -1;
private:
  std::vector<bool> solution_;
  bool solved_;
  ::Pumpkin::IEncoder::IFactory *encoder_factory_;

};
}

#endif // SIMPLESATSOLVER_SRC_SOLVER_WRAPPERS_PUMPKIN_H_
