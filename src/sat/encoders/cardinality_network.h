//
// Created by jens on 05-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_SAT_ENCODERS_CARDINALITY_NETWORK_H_
#define SIMPLESATSOLVER_SRC_SAT_ENCODERS_CARDINALITY_NETWORK_H_


#include "../sat_problem.h"
namespace simple_sat_solver::sat {
class CardinalityNetwork {
public:
  static CardinalityNetwork Encode(SatProblem &sat, std::vector<Lit> variables, int max);
  void print(std::vector<bool> sol);
private:
  CardinalityNetwork(SatProblem *sat);

  std::vector<Lit> HMerge(std::vector<Lit> a, std::vector<Lit> b);
  std::vector<Lit> HSort(std::vector<Lit> a);
  std::vector<Lit> SMerge(std::vector<Lit> a, std::vector<Lit> b);
  std::vector<Lit> Card(std::vector<Lit> a, int k);

  int I(int i);

  SatProblem * sat_;

  struct I2O {
    std::vector<Lit> a;
    std::vector<Lit> b;
    std::vector<Lit> d;
    std::vector<Lit> e;
    std::vector<Lit> o;
  };

  struct I1O {
    std::vector<Lit> a;
    std::vector<Lit> o;
  };


  // For debug
  std::vector<I2O> HMerge_;
  std::vector<I2O> SMerge_;
  std::vector<I1O> HSort_;
  void print_values(std::vector<bool> sol, std::vector<Lit> lits);
};
}

#endif // SIMPLESATSOLVER_SRC_SAT_ENCODERS_CARDINALITY_NETWORK_H_
