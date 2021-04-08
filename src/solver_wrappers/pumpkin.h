//
// Created by jens on 15-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_SOLVER_WRAPPERS_PUMPKIN_H_
#define SIMPLESATSOLVER_SRC_SOLVER_WRAPPERS_PUMPKIN_H_

#include "i_solver.h"
#include "../pumpkin/Propagators/Dynamic/Encoders/i_encoder.h"
#include "../pumpkin/Propagators/Dynamic/Encoders/propagator_encoder.h"
#include "../pumpkin/Basic Data Structures/problem_specification.h"
namespace Pumpkin {
class ProblemSpecification;
}
namespace simple_sat_solver::solver_wrappers {
class Pumpkin : public ISolver {
public:
  enum class CardinalityOption{ Totolizer, Sequential, Propagator};
  inline Pumpkin(::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory * encoder_factory, int start_upperbound) : optimisation_encoder_factory_(encoder_factory), constraint_encoder_factory_(encoder_factory),solved_(false), start_uppberboud_(start_upperbound) {};
  inline Pumpkin(::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory * optimisation_encoder_factory, ::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory * constraint_encoder_factory,int start_upperbound) : optimisation_encoder_factory_(optimisation_encoder_factory), constraint_encoder_factory_(constraint_encoder_factory),solved_(false), start_uppberboud_(start_upperbound) {};
  inline Pumpkin(::Pumpkin::IEncoder<::Pumpkin::CardinalityConstraint>::IFactory * encoder_factory, int start_upperbound) : optimisation_encoder_factory_(nullptr), solved_(false), start_uppberboud_(start_upperbound) {assert(false);};

  bool Solve(const sat::SatProblem &p) override;
  bool Optimize(const sat::SatProblem &p) override;
  ::Pumpkin::ProblemSpecification ConvertProblem(sat::SatProblem &p);
  std::vector<bool> GetSolution() const override;
  ~Pumpkin() override {
    if (constraint_encoder_factory_ == optimisation_encoder_factory_) {
      delete constraint_encoder_factory_;
      constraint_encoder_factory_ = nullptr;
      optimisation_encoder_factory_ = nullptr;
    } else {
      delete constraint_encoder_factory_;
      constraint_encoder_factory_ = nullptr;
      delete optimisation_encoder_factory_;
      optimisation_encoder_factory_ = nullptr;
    }
  }

  int start_uppberboud_ = -1;

  static std::vector<bool> check_solution_;
  static bool check_sol;
private:
  std::vector<bool> solution_;
  bool solved_;
  ::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory *constraint_encoder_factory_;
  ::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory *optimisation_encoder_factory_;

};
}

#endif // SIMPLESATSOLVER_SRC_SOLVER_WRAPPERS_PUMPKIN_H_
