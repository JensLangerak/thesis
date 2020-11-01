//
// Created by jens on 28-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_I_ENCODER_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_I_ENCODER_H_

#include <vector>
namespace Pumpkin {
class SolverState;
class BooleanLiteral;
class WatchedCardinalityConstraint;
class CardinalityConstraint;
class IEncoder {
public:
  virtual std::vector<std::vector<BooleanLiteral>>
  Encode(SolverState &state) = 0;
  virtual std::vector<std::vector<BooleanLiteral>>
  Encode(SolverState &state, std::vector<BooleanLiteral> lits);
  virtual ~IEncoder();
  virtual bool SupportsIncremental() { return false; };
  bool AddEncodingDynamic() { return add_dynamic_; };

  class IFactory {
  public:
    IEncoder *Create(std::vector<BooleanLiteral> variables, int min, int max);
    IEncoder *Create(WatchedCardinalityConstraint &constraint);
    IEncoder *Create(CardinalityConstraint &constraint);
    virtual ~IFactory();

    bool add_dynamic_ = true;

  protected:
    virtual IEncoder *CallConstructor(std::vector<BooleanLiteral> variables,
                                      int min, int max) = 0;
  };

protected:
  IEncoder(){};
  bool add_dynamic_ = false;
};
} // namespace Pumpkin

#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_I_ENCODER_H_
