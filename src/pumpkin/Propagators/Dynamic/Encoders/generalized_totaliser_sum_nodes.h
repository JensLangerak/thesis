//
// Created by jens on 16-03-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_ENCODERS_GENERALIZED_TOTALISER_SUM_NODES_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_ENCODERS_GENERALIZED_TOTALISER_SUM_NODES_H_


#include "../../../Utilities/boolean_literal.h"
#include "../../../Utilities/problem_specification.h"
#include "i_encoder.h"
#include <cassert>
#include <unordered_map>
#include <vector>

namespace Pumpkin {
class WatchedPbSumConstraint;
class GeneralizedTotaliserSumNodes : public IEncoder<PbSumConstraint> {
public:
  GeneralizedTotaliserSumNodes(std::vector<WeightedLiteral> inputs, std::vector<WeightedLiteral> outputs, int max);

//  void PrintInfo() override;
  void Encode(SolverState &state) override;
  void Encode(SolverState &state, std::vector<BooleanLiteral> lits) override;

  std::vector<WeightedLiteral> inputs_;
  std::vector<WeightedLiteral> outputs_;
  std::unordered_map<int, WeightedLiteral> weight_map_;
  int max_;
  int lowest_after_max_;
  int max_output_;
  GeneralizedTotaliserSumNodes* l_child;
  GeneralizedTotaliserSumNodes* r_child;
  bool UpdateMax(int max, SolverState &state) override;

  WeightedLiteral GetLiteral(int weight);

  class Factory : public IEncoder<PbSumConstraint>::IFactory {
  public:
    Factory(EncodingStrategy strategy, double delay_factor) : IEncoder<PbSumConstraint>::IFactory(strategy, delay_factor) {};
    ~Factory() override = default;

  protected:
    IEncoder<PbSumConstraint> * CallConstructor(PbSumConstraint &constraint) override;
  };


  WatchedPbSumConstraint *watched_constraint;
private:
  struct LitActiveData {
      BooleanLiteral literal;
      double activity;
      int weight;
      LitActiveData(BooleanLiteral l, double activiy, int weight) : literal(l), activity(activiy), weight(weight) {}
    };

 void CreateLeafs(SolverState &state);
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_ENCODERS_GENERALIZED_TOTALISER_SUM_NODES_H_
