//
// Created by jens on 06-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_SAT_ENCODERS_INCREMENTAL_ENCODING_H_
#define SIMPLESATSOLVER_SRC_SAT_ENCODERS_INCREMENTAL_ENCODING_H_

#include "../sat_problem.h"
namespace simple_sat_solver::sat {
class IncrementalEncoding {
public:
  static void Encode(SatProblem &sat, std::vector<Lit> variables,
                     std::vector<int> weights, int max);

private:
  IncrementalEncoding(SatProblem *sat, std::vector<Lit> variables,
                       std::vector<int> weights, int max);
  ~IncrementalEncoding();

  SatProblem *sat_;
  std::vector<int> weights_;
  std::vector<Lit> variables_;
  int max;
  std::vector<int> r_;
  std::vector<int> w_;
  void FindBases();
  void CreateNetwork();
  std::vector<Lit> EncodeBySorter(std::vector<Lit> vars);
  std::vector<Lit> Oe4Sel(std::vector<Lit> vars, int k);
  std::vector<Lit> y_;
  std::vector<Lit> z_;
  int ZIndex(int i, int w);
  std::vector<Lit> SelectOne(std::vector<Lit> vars);
};
} // namespace simple_sat_solver::sat

#endif // SIMPLESATSOLVER_SRC_SAT_ENCODERS_INCREMENTAL_ENCODING_H_
