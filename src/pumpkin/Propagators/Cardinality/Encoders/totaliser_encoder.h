//
// Created by jens on 05-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_TOTALISER_ENCODER_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_TOTALISER_ENCODER_H_

#include "i_encoder.h"
namespace Pumpkin {


class TotaliserEncoder : public IEncoder {

public:
  virtual std::vector<std::vector<BooleanLiteral>> Encode(SolverState &state) override;
//  static std::vector<std::vector<BooleanLiteral>> Encode(SolverState &state, std::vector<BooleanLiteral> variables, int min,
//                     int max);

  TotaliserEncoder(std::vector<BooleanLiteral> variables, int min,
                   int max);

  ~TotaliserEncoder() ;

  void SetSumLiterals(std::vector<BooleanLiteral> sum_lits) override;
  class Factory : public IEncoder::IFactory {
    IEncoder * CallConstructor(std::vector<BooleanLiteral> literals, int min, int max) override {return new TotaliserEncoder(literals, min, max);};
  };

  struct Node {
    int index;
    std::vector<BooleanLiteral> variables;
    std::vector<BooleanLiteral> counting_variables;
    Node *left;
    Node *right;
    Node() : left(nullptr),  right(nullptr) {}
    ~Node();
  };

  Node *CreateTree(std::vector<BooleanLiteral> variables);
  Node *CreateTree(std::vector<BooleanLiteral> variables, bool use_set_sum_lits);


  SolverState *solver_state_;
  std::vector<BooleanLiteral> variables_;
  std::vector<std::vector<BooleanLiteral>> added_clauses_;
  std::vector<BooleanLiteral> potential_sum_literals_;
  int min_;
  int max_;
  Node *root_;
  void SetMin();
  void SetMax();
};
} //
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_TOTALISER_ENCODER_H_
