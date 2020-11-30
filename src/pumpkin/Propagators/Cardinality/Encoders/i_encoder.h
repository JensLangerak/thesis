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
class SumConstraint;
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
  bool add_incremental;

  bool AddEncodingDynamic() { return add_dynamic_; };
  virtual bool EncodingAddAtStart() { return !add_dynamic_; };
  virtual bool EncodingAdded() { return encoding_added_;};
  virtual bool IsAdded(BooleanLiteral l);
  virtual void SetSumLiterals(std::vector<BooleanLiteral> sum_lits) {assert(true);};
  class IFactory {
  public:
    IEncoder *Create(std::vector<BooleanLiteral> variables, int min, int max);
    IEncoder *Create(WatchedCardinalityConstraint &constraint);
    IEncoder *Create(CardinalityConstraint &constraint);
    IEncoder *Create(SumConstraint & constraint); //TODO restucutre
    virtual ~IFactory();

    bool add_dynamic_ = true;
    bool add_incremetal_ = false;

  protected:
    virtual IEncoder *CallConstructor(std::vector<BooleanLiteral> variables,
                                      int min, int max) = 0;
  };

  int log_id_;
protected:
  IEncoder(){};
  bool add_dynamic_ = false;
  bool encoding_added_ = false;
};
} // namespace Pumpkin

#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_I_ENCODER_H_
