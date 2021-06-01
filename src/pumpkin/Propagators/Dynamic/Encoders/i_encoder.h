//
// Created by jens on 24-05-21.
//

#ifndef PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_ENCODERS_I_ENCODER_H_
#define PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_ENCODERS_I_ENCODER_H_
#include <cassert>
#include <vector>
#include "../../../Utilities/boolean_literal.h"

namespace Pumpkin {
class SolverState;
template <class T>
class IEncoder {
public:
  virtual void Encode(SolverState &state) = 0;
  virtual void Encode(SolverState &state, std::vector<BooleanLiteral> lits) = 0;

  virtual ~IEncoder() = default;
  double add_delay = 1;

  virtual bool EncodingAdded() { return encoding_added_;};
  virtual bool EncodingPartialAdded() { return partial_added_;};
  virtual bool IsAdded(BooleanLiteral l) { return encoding_added_;};

  enum EncodingStrategy {START, DYNAMIC, INCREMENTAL, NEVER};
  EncodingStrategy encoding_strategy_;
  class IFactory {
  public:
    IEncoder<T> *Create(T &constraint);

    virtual ~IFactory() = default;

    double add_delay_ =1.0;
    EncodingStrategy encoding_strategy_;
  protected:
    IFactory(EncodingStrategy encoding_strategy, double add_delay_);
    virtual IEncoder *CallConstructor(T &constraint) = 0;
  };

  virtual bool UpdateMax(int max, SolverState &state) = 0;

protected:
  IEncoder(){};

public:
  bool encoding_added_ = false;
  bool partial_added_ = false;
  int log_id_;
  virtual bool EncodingAddAtStart() { return encoding_strategy_ == START; }
};


} // namespace Pumpkin

#endif // PUMPKIN_PUBLIC_CODE_PUMPKIN_PROPAGATORS_DYNAMIC_ENCODERS_I_ENCODER_H_
