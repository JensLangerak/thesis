//
// Created by jens on 05-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_SAT_ENCODERS_GENERALIZED_TOTALISER_H_
#define SIMPLESATSOLVER_SRC_SAT_ENCODERS_GENERALIZED_TOTALISER_H_

#include "../sat_problem.h"
namespace simple_sat_solver::sat {

class GeneralizedTotaliser {

public:
  static void Encode(SatProblem &sat, std::vector<Lit> variables,
                     std::vector<int> weights, int max);

private:
  GeneralizedTotaliser(SatProblem *sat, std::vector<Lit> variables,
                       std::vector<int> weights, int max);
  ~GeneralizedTotaliser();

  struct Node {
    int index;
    std::vector<Lit> variables;
    std::vector<Lit> counting_variables;
    std::vector<int> counting_variables_weights;
    std::vector<int> values_map;
    std::vector<int> variables_weights;

    int max;
    Node *left;
    Node *right;
    Node() : left(nullptr), right(nullptr) {}
    ~Node();
  };

  Node *CreateTree(std::vector<Lit> variables, std::vector<int> weights);

  SatProblem *sat_;
  std::vector<Lit> variables_;
  std::vector<int> weights_;
  int max_;
  Node *root_;
  void SetMax();
  int GetValueIndex(Node *node, int value);
};
} // namespace simple_sat_solver::sat
#endif // SIMPLESATSOLVER_SRC_SAT_ENCODERS_GENERALIZED_TOTALISER_H_
