//
// Created by jens on 31-05-21.
//

#ifndef SIMPLESATSOLVER_SRC_BENCHMARK_BENCHMARK_H_
#define SIMPLESATSOLVER_SRC_BENCHMARK_BENCHMARK_H_

#include "../pumpkin/Propagators/i_constraint_adder.h"
#include "../pumpkin/Utilities/problem_specification.h"
#include "../pumpkin/Utilities/solver_output.h"
#include "../solver_wrappers/pumpkin.h"
#include <string>
namespace simple_sat_solver::benchmark {
enum class SolverType {
  ENCODER,
  DYNAMIC,
  INCREMENTAL,
  PROPAGATOR
};
class Benchmark {
public:
  static void BenchmarkMain(int argc, char *argv[], Benchmark * benchmark);
  static void BenchmarkMain(Benchmark *benchmark);
  SolverType solver_type_ = SolverType::ENCODER;
  double delay_factor_ = 0;
  std::string problem_file_full_path_;
  std::string log_dir_;
  int start_penalty_ = 10000;
  int solve_time_ = 60;
protected:
  Benchmark()= default;
  void Init(int argc, char **argv);
  void WriteHeader();
  std::string GetEncoderName();
  virtual Pumpkin::ProblemSpecification *GetProblem() = 0;
  virtual Pumpkin::IConstraintAdder<Pumpkin::PseudoBooleanConstraint> *
  CreatePbConstraintWrapper();
  virtual void CheckSolutionCorrectness(Pumpkin::ProblemSpecification *specification,
                           Pumpkin::SolverOutput output) {};
};
}

#endif // SIMPLESATSOLVER_SRC_BENCHMARK_BENCHMARK_H_
