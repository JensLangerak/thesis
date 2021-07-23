//
// Created by jens on 31-05-21.
//

#ifndef SIMPLESATSOLVER_SRC_BENCHMARK_BENCHMARK_H_
#define SIMPLESATSOLVER_SRC_BENCHMARK_BENCHMARK_H_

#include "../pumpkin/Propagators/i_constraint_adder.h"
#include "../pumpkin/Utilities/problem_specification.h"
#include "../pumpkin/Utilities/solver_output.h"
#include "../solver_wrappers/pumpkin.h"
#include "benchmark.h"
#include <string>
namespace simple_sat_solver::benchmark {
enum class SolverType {
  ENCODER,
  DYNAMIC,
  INCREMENTAL,
  PROPAGATOR,
  PAIRS,
  TOPDOWN,
  BOTTOMLAYERS
};
class Benchmark {
public:
  void Main(int argc, char *argv[]);
  void Main();
  void MainOrder();
  virtual ~Benchmark() = default;
  SolverType solver_type_ = SolverType::ENCODER;
  double delay_factor_ = 0;
  std::string problem_file_full_path_;
  std::string log_dir_;
  int start_penalty_ = 10000;
  int solve_time_ = 600;
  int probe_time_ = 10;
protected:
  Benchmark()= default;
  void Init(int argc, char **argv);
  void WriteHeader();
  std::string GetEncoderName();
  virtual Pumpkin::ProblemSpecification GetProblem() = 0;
  virtual Pumpkin::IConstraintAdder<Pumpkin::PseudoBooleanConstraint> *
  CreatePbConstraintWrapper(SolverType solver_type);
  virtual void CheckSolutionCorrectness(Pumpkin::ProblemSpecification *specification,
                           Pumpkin::SolverOutput output);
  Pumpkin::ProblemSpecification ConvertSatToPumpkin(sat::SatProblem *problem);
  Pumpkin::IConstraintAdder<Pumpkin::PseudoBooleanConstraint> *optimisation_adder_;
  Pumpkin::IConstraintAdder<Pumpkin::PseudoBooleanConstraint> *pb_adder_;
  Pumpkin::ProblemSpecification
  CreateProblemOrderLiterals(Pumpkin::ProblemSpecification specification,
                             Pumpkin::SolverState &state);
  void ParseArgument(std::string basic_string);
  void ParseKey(std::string basic_string);
  void ParseKeyValue(std::string basic_string, std::string basic_string_1);
  void OrderLitUsingHamming(std::vector<Pumpkin::BooleanLiteral> &output_lits,
                            std::vector<uint32_t> &output_weights,
                            Pumpkin::SolverState &state,
                            std::vector<Pumpkin::BooleanLiteral> input_lits,
                            std::vector<uint32_t> input_weights);
};
}

#endif // SIMPLESATSOLVER_SRC_BENCHMARK_BENCHMARK_H_
