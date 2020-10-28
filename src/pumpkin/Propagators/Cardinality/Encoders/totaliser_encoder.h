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

private:
  struct Node2 {
    int index;
    std::vector<BooleanLiteral> variables;
    std::vector<BooleanLiteral> counting_variables;
    Node2 *left;
    Node2 *right;
    Node2() : left(nullptr),  right(nullptr) {}
    ~Node2();
  };

  Node2 *CreateTree(std::vector<BooleanLiteral> variables);


  SolverState *solver_state_;
  std::vector<BooleanLiteral> variables_;
  std::vector<std::vector<BooleanLiteral>> added_clauses_;
  int min_;
  int max_;
  Node2 *root_;
  void SetMin();
  void SetMax();
};
} //
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_TOTALISER_ENCODER_H_
