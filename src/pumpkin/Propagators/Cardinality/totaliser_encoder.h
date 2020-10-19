//
// Created by jens on 05-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_SAT_ENCODERS_TOTALISER_ENCODER_H_
#define SIMPLESATSOLVER_SRC_SAT_ENCODERS_TOTALISER_ENCODER_H_

#include "../../Engine/solver_state.h"
namespace Pumpkin {


class TotaliserEncoder {

public:
  static void Encode(SolverState &state, std::vector<BooleanLiteral> variables, int min,
                     int max);

private:
  TotaliserEncoder(SolverState &state, std::vector<BooleanLiteral> variables, int min,
                   int max);
  ~TotaliserEncoder();

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


  SolverState *solver_state_;
  std::vector<BooleanLiteral> variables_;
  int min_;
  int max_;
  Node *root_;
  void SetMin();
  void SetMax();
};
} //
#endif // SIMPLESATSOLVER_SRC_SAT_ENCODERS_TOTALISER_ENCODER_H_
