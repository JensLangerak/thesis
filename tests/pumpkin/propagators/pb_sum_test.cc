//
// Created by jens on 26-03-21.
//

#include "lib/catch2/catch.hpp"
#include "src/sat/encoders/totaliser_encoder.h"
#include "src/solver_wrappers/pumpkin.h"
#include <src/pumpkin/Basic Data Structures/solver_parameters.h>
#include <src/pumpkin/Engine/constraint_satisfaction_solver.h>
#include <src/pumpkin/Propagators/Dynamic/Encoders/generalized_totliser_sum_root.h>

namespace Pumpkin {
using namespace simple_sat_solver;
using namespace simple_sat_solver::sat;
TEST_CASE("Simple pumpkin test ", "[pb_sum]") {
  ProblemSpecification problem;
  problem.num_Boolean_variables_ = 5;

  std::vector<BooleanLiteral> clause = {
      BooleanLiteral(BooleanVariable(1), true),
      BooleanLiteral(BooleanVariable(2), false)};
  problem.AddClause(clause);
  SolverParameters params;
  params.bump_decision_variables = true;
  ConstraintSatisfactionSolver solver(problem, params);
  SolverOutput solver_output = solver.Solve(3);

  SECTION("T1") { REQUIRE(solver_output.HasSolution()); }
}

TEST_CASE("Unweighted sum test", "[pb_sum]") {
  ProblemSpecification problem;
  problem.num_Boolean_variables_ = 10;

  std::vector<BooleanLiteral> inputs;
  std::vector<uint32_t> input_w;
  for (int i = 1; i <= 5; ++i) {
    inputs.push_back(BooleanLiteral(BooleanVariable(i), true));
    input_w.push_back(1);
  }

  SECTION("Enough outputs") {
    std::vector<BooleanLiteral> outputs;
    std::vector<uint32_t> output_w;
    for (int i = 6; i <= 10; ++i) {
      outputs.push_back(BooleanLiteral(BooleanVariable(i), true));
      output_w.push_back(i - 5);
    }
    auto encoder =
        (::Pumpkin::IEncoder<::Pumpkin::PbSumConstraint>::IFactory
             *)new ::Pumpkin::GeneralizedTotaliserSumNodes::Factory();
    PbSumConstraint constraint(inputs, input_w, outputs, output_w, encoder);
    problem.pb_sum_constraints_.push_back(constraint);
    SolverParameters params;
    params.bump_decision_variables = true;

    SECTION("None set") {
      ConstraintSatisfactionSolver solver(problem, params);
      SolverOutput solver_output = solver.Solve(3);
      REQUIRE(solver_output.HasSolution());
    }
    SECTION("All inputs set") {
      SECTION("All to true") {
        for (int i = 1; i <= 5; ++i) {
          problem.AddUnaryClause(BooleanLiteral(BooleanVariable(i), true));
        }

        ConstraintSatisfactionSolver solver(problem, params);
        SolverOutput solver_output = solver.Solve(3);
        REQUIRE(solver_output.HasSolution());
        auto solution = solver_output.solution;
        REQUIRE(solution.size() == 11);
        for (int i = 6; i <= 10; ++i) {
          REQUIRE(solution[i]);
        }
      }
      SECTION("All to false") {
        for (int i = 1; i <= 5; ++i) {
          problem.AddUnaryClause(BooleanLiteral(BooleanVariable(i), false));
        }

        ConstraintSatisfactionSolver solver(problem, params);
        SolverOutput solver_output = solver.Solve(3);
        REQUIRE(solver_output.HasSolution());
        auto solution = solver_output.solution;
        REQUIRE(solution.size() == 11);
        for (int i = 6; i <= 10; ++i) {
          REQUIRE_FALSE(solution[i]);
        }
      }
      SECTION("Half true") {
        for (int i = 1; i <= 5; ++i) {
          problem.AddUnaryClause(
              BooleanLiteral(BooleanVariable(i), (i % 2) == 0));
        }

        ConstraintSatisfactionSolver solver(problem, params);
        SolverOutput solver_output = solver.Solve(3);
        REQUIRE(solver_output.HasSolution());
        auto solution = solver_output.solution;
        REQUIRE(solution.size() == 11);
        for (int i = 6; i <= 7; ++i) {
          REQUIRE(solution[i]);
        }
        for (int i = 8; i <= 10; ++i) {
          //        REQUIRE_FALSE(solution[i]); // TODO actually not required
        }
      }
      SECTION("Half true tight") {
        for (int i = 1; i <= 5; ++i) {
          problem.AddUnaryClause(
              BooleanLiteral(BooleanVariable(i), (i % 2) == 0));
        }
        problem.AddUnaryClause(BooleanLiteral(BooleanVariable(8), false));

        ConstraintSatisfactionSolver solver(problem, params);
        SolverOutput solver_output = solver.Solve(3);
        REQUIRE(solver_output.HasSolution());
        auto solution = solver_output.solution;
        REQUIRE(solution.size() == 11);
        for (int i = 6; i <= 7; ++i) {
          REQUIRE(solution[i]);
        }
        for (int i = 8; i <= 10; ++i) {
          REQUIRE_FALSE(solution[i]); // TODO actually not required
        }
      }
      SECTION("Not solvable") {
        for (int i = 1; i <= 5; ++i) {
          problem.AddUnaryClause(
              BooleanLiteral(BooleanVariable(i), (i % 2) == 0));
        }
        problem.AddUnaryClause(BooleanLiteral(BooleanVariable(7), false));
        ConstraintSatisfactionSolver solver(problem, params);
        SolverOutput solver_output = solver.Solve(3);
        REQUIRE_FALSE(solver_output.HasSolution());
      }

      SECTION("Solvable") {
        for (int i = 1; i <= 5; ++i) {
          problem.AddUnaryClause(
              BooleanLiteral(BooleanVariable(i), (i % 2) == 0));
        }
        problem.AddUnaryClause(BooleanLiteral(BooleanVariable(7), true));
        ConstraintSatisfactionSolver solver(problem, params);
        SolverOutput solver_output = solver.Solve(3);
        REQUIRE(solver_output.HasSolution());
      }
      SECTION("Solvable larger sum") {
        for (int i = 1; i <= 5; ++i) {
          problem.AddUnaryClause(
              BooleanLiteral(BooleanVariable(i), (i % 2) == 0));
        }
        problem.AddUnaryClause(BooleanLiteral(BooleanVariable(9), true));
        ConstraintSatisfactionSolver solver(problem, params);
        SolverOutput solver_output = solver.Solve(3);
        REQUIRE(solver_output.HasSolution());
      }
      SECTION("Not solvable 2") {
        for (int i = 1; i <= 5; ++i) {
          problem.AddUnaryClause(BooleanLiteral(BooleanVariable(i), true));
        }
        problem.AddUnaryClause(BooleanLiteral(BooleanVariable(7), false));
        ConstraintSatisfactionSolver solver(problem, params);
        SolverOutput solver_output = solver.Solve(3);
        REQUIRE_FALSE(solver_output.HasSolution());
      }
    }
  }

  SECTION("Not enough outputs") {
    std::vector<BooleanLiteral> outputs;
    std::vector<uint32_t> output_w;
    for (int i = 6; i <= 8; ++i) {
      outputs.push_back(BooleanLiteral(BooleanVariable(i), true));
      output_w.push_back(i - 5);
    }
    auto encoder =
        (::Pumpkin::IEncoder<::Pumpkin::PbSumConstraint>::IFactory
             *)new ::Pumpkin::GeneralizedTotaliserSumNodes::Factory();
    PbSumConstraint constraint(inputs, input_w, outputs, output_w, encoder);
    problem.pb_sum_constraints_.push_back(constraint);
    SolverParameters params;
    params.bump_decision_variables = true;

    for (int i = 1; i <= 5; ++i) {
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(i), true));
    }
    SECTION("Last set to false") {
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(8), false));
      ConstraintSatisfactionSolver solver(problem, params);
      SolverOutput solver_output = solver.Solve(3);
      REQUIRE_FALSE(solver_output.HasSolution());
      //    REQUIRE(solver_output.solution[8]);
    }
    //    SECTION("Last not set") {
    //      ConstraintSatisfactionSolver solver(problem, params);
    //      REQUIRE_THROWS(solver.Solve(3));
    //    }
  }
  SECTION("Gaps in output") {
    std::vector<BooleanLiteral> outputs;
    std::vector<uint32_t> output_w;
    for (int i = 6; i <= 7; ++i) {
      outputs.push_back(BooleanLiteral(BooleanVariable(i), true));
      output_w.push_back(i - 5);
    }
    outputs.push_back(BooleanLiteral(BooleanVariable(8), true));
    output_w.push_back(10);
    auto encoder =
        (::Pumpkin::IEncoder<::Pumpkin::PbSumConstraint>::IFactory
             *)new ::Pumpkin::GeneralizedTotaliserSumNodes::Factory();
    PbSumConstraint constraint(inputs, input_w, outputs, output_w, encoder);
    problem.pb_sum_constraints_.push_back(constraint);
    SolverParameters params;
    params.bump_decision_variables = true;

    for (int i = 1; i <= 5; ++i) {
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(i), true));
    }
    problem.AddUnaryClause(BooleanLiteral(BooleanVariable(8), false));
    ConstraintSatisfactionSolver solver(problem, params);
    SolverOutput solver_output = solver.Solve(3);
    REQUIRE(solver_output.HasSolution());
    REQUIRE(solver_output.solution[7]);
    REQUIRE_FALSE(solver_output.solution[8]);
  }
}
TEST_CASE("Weighted sum test, all same weight", "[pb_sum]") {
  ProblemSpecification problem;
  problem.num_Boolean_variables_ = 5;

  std::vector<BooleanLiteral> inputs;
  std::vector<uint32_t> input_w;
  for (int i = 1; i <= 5; ++i) {
    inputs.push_back(BooleanLiteral(BooleanVariable(i), true));
    input_w.push_back(3);
  }
  std::vector<BooleanLiteral> outputs;
  std::vector<uint32_t> output_w;
  SECTION("Output weights 1") {
    problem.num_Boolean_variables_ += 3 * 5;
    for (int i = 6; i <= (3 * 5) + 5; i++) {
      outputs.push_back(BooleanLiteral(BooleanVariable(i), true));
      output_w.push_back(i - 5);
    }

    auto encoder =
        (::Pumpkin::IEncoder<::Pumpkin::PbSumConstraint>::IFactory
             *)new ::Pumpkin::GeneralizedTotaliserSumNodes::Factory();
    PbSumConstraint constraint(inputs, input_w, outputs, output_w, encoder);
    problem.pb_sum_constraints_.push_back(constraint);
    SolverParameters params;
    params.bump_decision_variables = true;
    SECTION("All to true") {
      for (int i = 1; i <= 5; ++i) {
        problem.AddUnaryClause(BooleanLiteral(BooleanVariable(i), true));
      }
      ConstraintSatisfactionSolver solver(problem, params);
      SolverOutput solver_output = solver.Solve(3);
      REQUIRE(solver_output.HasSolution());
      auto solution = solver_output.solution;
      for (int i = 6; i <= problem.num_Boolean_variables_; ++i) {
        if ((i - 5) % 3 == 0)
          REQUIRE(solution[i]);
      }
    }
    SECTION("Gaps set to false") {
      for (int i = 1; i <= 5; ++i) {
        problem.AddUnaryClause(BooleanLiteral(BooleanVariable(i), true));
      }
      for (int i = 6; i <= problem.num_Boolean_variables_; ++i) {
        if ((i - 5) % 3 != 0)
          problem.AddUnaryClause(BooleanLiteral(BooleanVariable(i), false));
      }
      ConstraintSatisfactionSolver solver(problem, params);
      SolverOutput solver_output = solver.Solve(3);
      REQUIRE(solver_output.HasSolution());
      auto solution = solver_output.solution;
      for (int i = 6; i <= problem.num_Boolean_variables_; ++i) {
        if ((i - 5) % 3 == 0)
          REQUIRE(solution[i]);
      }
    }
    SECTION("Output set to false") {
      for (int i = 1; i <= 5; ++i) {
        problem.AddUnaryClause(BooleanLiteral(BooleanVariable(i), true));
      }
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(5 + 2 * 3), false));
      ConstraintSatisfactionSolver solver(problem, params);
      SolverOutput solver_output = solver.Solve(3);
      REQUIRE_FALSE(solver_output.HasSolution());
    }
  }
  SECTION("Output weight only possible weights") {
    problem.num_Boolean_variables_ += 1 * 5;
    for (int i = 6; i <= (1 * 5) + 5; i++) {
      outputs.push_back(BooleanLiteral(BooleanVariable(i), true));
      output_w.push_back((i - 5) * 3);
    }
    auto encoder =
        (::Pumpkin::IEncoder<::Pumpkin::PbSumConstraint>::IFactory
             *)new ::Pumpkin::GeneralizedTotaliserSumNodes::Factory();
    PbSumConstraint constraint(inputs, input_w, outputs, output_w, encoder);
    problem.pb_sum_constraints_.push_back(constraint);
    SolverParameters params;
    params.bump_decision_variables = true;

    SECTION("All to true") {
      for (int i = 1; i <= 5; ++i) {
        problem.AddUnaryClause(BooleanLiteral(BooleanVariable(i), true));
      }
      ConstraintSatisfactionSolver solver(problem, params);
      SolverOutput solver_output = solver.Solve(3);
      REQUIRE(solver_output.HasSolution());
      auto solution = solver_output.solution;
      for (int i = 6; i <= problem.num_Boolean_variables_; ++i) {
        REQUIRE(solution[i]);
      }
    }
  }
}
TEST_CASE("Weighted sum test, different weights", "[pb_sum]") {
  ProblemSpecification problem;
  problem.num_Boolean_variables_ = 5;

  std::vector<BooleanLiteral> inputs;
  std::vector<uint32_t> input_w;

  for (int i = 1; i <= 5; ++i) {
    inputs.push_back(BooleanLiteral(BooleanVariable(i), true));
    input_w.push_back(i);
  }
  std::vector<BooleanLiteral> outputs;
  std::vector<uint32_t> output_w;
  int max_sum = 5 + 4 + 3 + 2 + 1;
  SECTION("Single output") {
    outputs.push_back(BooleanLiteral(BooleanVariable(6), true));
    output_w.push_back(12);

    problem.num_Boolean_variables_++;
    auto encoder =
        (::Pumpkin::IEncoder<::Pumpkin::PbSumConstraint>::IFactory
        *)new ::Pumpkin::GeneralizedTotaliserSumNodes::Factory();
    encoder->add_dynamic_ = true;
    PbSumConstraint constraint(inputs, input_w, outputs, output_w, encoder);
    problem.pb_sum_constraints_.push_back(constraint);
    SolverParameters params;
    params.bump_decision_variables = true;
    SECTION("Nothing set") {
      ConstraintSatisfactionSolver solver(problem, params);
      SolverOutput solver_output = solver.Solve(3);
      REQUIRE(solver_output.HasSolution());
    }

    SECTION("Less than set") {
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(2), true));
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(4), true));
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(5), true));
      ConstraintSatisfactionSolver solver(problem, params);
      SolverOutput solver_output = solver.Solve(3);
      REQUIRE(solver_output.HasSolution());
    }
    SECTION("One less set") {
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(2), true));
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(4), true));
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(5), true));
      SECTION("Output not set") {
        ConstraintSatisfactionSolver solver(problem, params);
        SolverOutput solver_output = solver.Solve(3);
        REQUIRE(solver_output.HasSolution());
      }
      SECTION("Output false") {
        ConstraintSatisfactionSolver solver(problem, params);
        problem.AddUnaryClause(BooleanLiteral(BooleanVariable(6), false));
        SolverOutput solver_output = solver.Solve(3);
        REQUIRE(solver_output.HasSolution());
      }
    }
    SECTION("Exact set") {
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(3), true));
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(4), true));
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(5), true));
      SECTION("Output not set") {
        ConstraintSatisfactionSolver solver(problem, params);
        SolverOutput solver_output = solver.Solve(3);
        REQUIRE(solver_output.HasSolution());
      }
      SECTION("Output false") {
        problem.AddUnaryClause(BooleanLiteral(BooleanVariable(6), false));
        ConstraintSatisfactionSolver solver(problem, params);
        SolverOutput solver_output = solver.Solve(3);
        REQUIRE_FALSE(solver_output.HasSolution());
      }
    }

  }

  SECTION("Output weights 1") {
    for (int i = 1; i <= max_sum; ++i) {
      outputs.push_back(BooleanLiteral(BooleanVariable(5 + i), true));
      output_w.push_back(i);
    }

    problem.num_Boolean_variables_ = 5 + max_sum;
    auto encoder =
        (::Pumpkin::IEncoder<::Pumpkin::PbSumConstraint>::IFactory
             *)new ::Pumpkin::GeneralizedTotaliserSumNodes::Factory();
    encoder->add_dynamic_ = true;
    PbSumConstraint constraint(inputs, input_w, outputs, output_w, encoder);
    problem.pb_sum_constraints_.push_back(constraint);
    SolverParameters params;
    params.bump_decision_variables = true;

    SECTION("All true") {
      for (int i = 1; i <= 5; ++i) {
        problem.AddUnaryClause(BooleanLiteral(BooleanVariable(i), true));
      }

      ConstraintSatisfactionSolver solver(problem, params);
      SolverOutput solver_output = solver.Solve(3);
      REQUIRE(solver_output.HasSolution());
      auto solution = solver_output.solution;
      for (int i = 1; i <= max_sum; ++i) {
        REQUIRE(solution[5 + i]);
      }
    }

    SECTION("Some true") {
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(2), true));
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(4), true));
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(5), true));

      SECTION("nothin else set") {
        ConstraintSatisfactionSolver solver(problem, params);
        SolverOutput solver_output = solver.Solve(3);
        REQUIRE(solver_output.HasSolution());
        auto solution = solver_output.solution;
        REQUIRE(solution[5 + 2]);
        REQUIRE(solution[5 + 4]);
        REQUIRE(solution[5 + 5]);
        REQUIRE(solution[5 + 4 + 2]);
        REQUIRE(solution[5 + 5 + 2]);
        REQUIRE(solution[5 + 5 + 4]);
        REQUIRE(solution[5 + 5 + 4 + 2]);
      }
      SECTION("Set to false") {
        problem.AddUnaryClause(BooleanLiteral(BooleanVariable(5 + 1), false));
        problem.AddUnaryClause(BooleanLiteral(BooleanVariable(5 + 3), false));
        problem.AddUnaryClause(BooleanLiteral(BooleanVariable(5 + 8), false));
        problem.AddUnaryClause(BooleanLiteral(BooleanVariable(5 + 12), false));
        problem.AddUnaryClause(BooleanLiteral(BooleanVariable(5 + 13), false));
        problem.AddUnaryClause(BooleanLiteral(BooleanVariable(5 + 14), false));
        problem.AddUnaryClause(BooleanLiteral(BooleanVariable(5 + 15), false));
        assert(max_sum == 15);
        ConstraintSatisfactionSolver solver(problem, params);
        SolverOutput solver_output = solver.Solve(3);
        REQUIRE(solver_output.HasSolution());
        auto solution = solver_output.solution;
        REQUIRE(solution[5 + 2]);
        REQUIRE(solution[5 + 4]);
        REQUIRE(solution[5 + 5]);
        REQUIRE(solution[5 + 4 + 2]);
        REQUIRE(solution[5 + 5 + 2]);
        REQUIRE(solution[5 + 5 + 4]);
        REQUIRE(solution[5 + 5 + 4 + 2]);
      }
      SECTION("Set subsum to false") {
        problem.AddUnaryClause(BooleanLiteral(BooleanVariable(5 + 6), false));
        ConstraintSatisfactionSolver solver(problem, params);
        SolverOutput solver_output = solver.Solve(3);
        REQUIRE_FALSE(solver_output.HasSolution());
      }
    }
  }
}
TEST_CASE("Pb max encoding sum encoding", "[pb_sum]") {
  ProblemSpecification problem;
  problem.num_Boolean_variables_ = 5;

  std::vector<BooleanLiteral> inputs;
  std::vector<uint32_t> input_w;

  for (int i = 1; i <= 5; ++i) {
    inputs.push_back(BooleanLiteral(BooleanVariable(i), true));
    input_w.push_back(i);
  }
  auto encoder =
      (::Pumpkin::IEncoder<::Pumpkin::PseudoBooleanConstraint>::IFactory
      *)new ::Pumpkin::GeneralizedTotliserSumRoot::Factory();
  encoder->add_dynamic_ = false;

  SECTION("Test limit") {
    PseudoBooleanConstraint pb =
        PseudoBooleanConstraint(inputs, input_w, 12, encoder);
    problem.pseudo_boolean_constraints_.push_back(pb);
    SolverParameters params;
    params.bump_decision_variables = true;

    SECTION("All true") {
      for (int i = 1; i <= 5; ++i) {
        problem.AddUnaryClause(BooleanLiteral(BooleanVariable(i), true));
      }

      ConstraintSatisfactionSolver solver(problem, params);
      SolverOutput solver_output = solver.Solve(3);
      REQUIRE_FALSE(solver_output.HasSolution());
    }
    SECTION("One over") {
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(5), true));
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(4), true));
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(3), true));
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(1), true));

      ConstraintSatisfactionSolver solver(problem, params);
      SolverOutput solver_output = solver.Solve(3);
      REQUIRE_FALSE(solver_output.HasSolution());
    }
    SECTION("Exact over") {
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(5), true));
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(4), true));
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(3), true));

      ConstraintSatisfactionSolver solver(problem, params);
      SolverOutput solver_output = solver.Solve(3);
      REQUIRE(solver_output.HasSolution());
    }
    SECTION("Exact rest false") {
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(5), true));
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(4), true));
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(3), true));
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(2), false));
      problem.AddUnaryClause(BooleanLiteral(BooleanVariable(1), false));

      ConstraintSatisfactionSolver solver(problem, params);
      SolverOutput solver_output = solver.Solve(3);
      REQUIRE(solver_output.HasSolution());
    }
  }
}
}