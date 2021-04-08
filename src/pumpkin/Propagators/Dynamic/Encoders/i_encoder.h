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
struct WeightedLiteral;
template <class T>
class IEncoder {
public:
  virtual void PrintInfo() { };
  virtual std::vector<std::vector<BooleanLiteral>>
  Encode(SolverState &state) = 0;
  virtual std::vector<std::vector<BooleanLiteral>>
  Encode(SolverState &state, std::vector<BooleanLiteral> lits);
  virtual std::vector<std::vector<BooleanLiteral>> Propagate(SolverState &state, std::vector<BooleanLiteral> reason, std::vector<BooleanLiteral> propage_values) { assert(false);};
  virtual void DebugInfo(SolverState & state);
  virtual bool GetLabel(BooleanLiteral l, std::string & label) {return false;};

  virtual ~IEncoder();
  virtual bool SupportsIncremental() { return false; };
  bool add_incremental;
  double add_delay = 1;

  virtual bool AddOnRestart() { return false;};
  virtual bool AddEncodingDynamic() { return add_dynamic_; };
  virtual bool EncodingAddAtStart() { return !add_dynamic_; };
  virtual bool EncodingAdded() { return encoding_added_;};
  virtual bool EncodingPartialAdded() { return partial_added_;};
  virtual bool IsAdded(BooleanLiteral l);

  virtual std::vector<WeightedLiteral> GetCurrentSumSet() { assert(false);};
//  virtual void SetSumLiterals(std::vector<BooleanLiteral> sum_lits) {assert(true);};
  class IFactory {
  public:
//    IEncoder *Create(std::vector<BooleanLiteral> variables, int min, int max);
//    IEncoder *Create(WatchedCardinalityConstraint &constraint);
    IEncoder<T> *Create(T &constraint);
//    IEncoder *Create(SumConstraint & constraint); //TODO restucutre
//    IEncoder *Create(PseudoBooleanConstraint & constraint); //TODO restucutre

    virtual ~IFactory();

    bool add_dynamic_ = true;
    bool add_incremetal_ = false;
    double add_delay_ =1.0;
  protected:
    virtual IEncoder *CallConstructor(T &constraint) = 0;
  };

  int log_id_;
  virtual bool UpdateMax(int max, SolverState &state) {return true;}; //TODO

protected:
  IEncoder(){};

public:
  bool add_dynamic_ = false;
  bool encoding_added_ = false;
  bool partial_added_ = false;
};


} // namespace Pumpkin

#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_I_ENCODER_H_
