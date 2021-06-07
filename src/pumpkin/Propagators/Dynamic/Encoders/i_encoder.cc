//
// Created by jens on 24-05-21.
//

#include "i_encoder.h"
#include "../../../Utilities/problem_specification.h"
namespace Pumpkin {
template <class T> IEncoder<T> *IEncoder<T>::IFactory::Create(T &constraint) {
  IEncoder * encoder = CallConstructor(constraint);
  encoder->add_delay = this->add_delay_;
  encoder->encoding_strategy_ = this->encoding_strategy_;
  return encoder;
}
template <class T>
IEncoder<T>::IFactory::IFactory(IEncoder::EncodingStrategy encoding_strategy,
                                double add_delay_) : encoding_strategy_(encoding_strategy), add_delay_(add_delay_) {}

//template class IEncoder<CardinalityConstraint>;
template class IEncoder<PseudoBooleanConstraint>;
//template class IEncoder<SumConstraint>;
//template class IEncoder<PbSumConstraint>;
}
