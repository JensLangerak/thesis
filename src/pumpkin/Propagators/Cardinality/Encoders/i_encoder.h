//
// Created by jens on 28-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_I_ENCODER_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_I_ENCODER_H_

#include <cassert>
#include <vector>

#include "../../../Basic Data Structures/boolean_literal.h"
namespace Pumpkin {
class SolverState;
class WatchedCardinalityConstraint;
class CardinalityConstraint;
class IEncoder {
public:
  virtual void PrintInfo() { };
  virtual std::vector<std::vector<BooleanLiteral>>
  Encode(SolverState &state) = 0;
  virtual std::vector<std::vector<BooleanLiteral>>
  Encode(SolverState &state, std::vector<BooleanLiteral> lits);
  virtual std::vector<std::vector<BooleanLiteral>> Propagate(SolverState &state, std::vector<BooleanLiteral> reason, std::vector<BooleanLiteral> propage_values) { assert(false);};

  virtual ~IEncoder();
  virtual bool SupportsIncremental() { return false; };
  bool AddEncodingDynamic() { return add_dynamic_; };
  virtual bool EncodingAdded() { return encoding_added_;};
  virtual bool IsAdded(BooleanLiteral l);
  WatchedCardinalityConstraint * cardinality_constraint_;
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

  virtual void RepairReasons(SolverState &state);

protected:
  IEncoder(){};
  bool add_dynamic_ = false;
  bool encoding_added_ = false;
};
} // namespace Pumpkin

#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_I_ENCODER_H_
