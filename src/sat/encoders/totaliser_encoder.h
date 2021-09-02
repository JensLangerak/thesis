//
// Created by jens on 05-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_SAT_ENCODERS_TOTALISER_ENCODER_H_
#define SIMPLESATSOLVER_SRC_SAT_ENCODERS_TOTALISER_ENCODER_H_

#include <vector>
namespace simple_sat_solver::sat {

class SatProblem;
class Lit;

class TotaliserEncoder {

public:
  static  void Encode(SatProblem &sat, std::vector<Lit> variables, int min,
                     int max);

private:
  TotaliserEncoder(SatProblem *sat, std::vector<Lit> variables, int min,
                   int max);
  ~TotaliserEncoder();

  struct Node {
    int index;
    std::vector<Lit> variables;
    std::vector<Lit> counting_variables;
    Node *left;
    Node *right;
    Node() : left(nullptr),  right(nullptr) {}
    ~Node();
  };

  Node *CreateTree(std::vector<Lit> variables);


  SatProblem *sat_;
  std::vector<Lit> variables_;
  int min_;
  int max_;
  Node *root_;
  void SetMin();
  void SetMax();
};
} // namespace simple_sat_solver::sat
#endif // SIMPLESATSOLVER_SRC_SAT_ENCODERS_TOTALISER_ENCODER_H_
