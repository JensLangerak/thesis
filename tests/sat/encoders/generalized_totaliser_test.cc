//
// Created by jens on 05-10-20.
//
#include "lib/catch2/catch.hpp"
#include "src/sat/encoders/generalized_totaliser.h"
#include "src/solver_wrappers/simple_solver.h"

namespace simple_sat_solver::sat {
TEST_CASE("Generalize totaliser: Min max bounds", "[encoder]") {
  solver_wrappers::SimpleSolver solver;
  SatProblem sat(3);
  std::vector<Lit> vars = {Lit(0), Lit(1), Lit(2)};
  std::vector<int> weights = {1,1,1};
  SECTION("Max tight bounds") {
    GeneralizedTotaliser::Encode(sat, vars, weights, 3);
    REQUIRE(solver.Solve(sat));
  }
  SECTION("Large max bound") {
    GeneralizedTotaliser::Encode(sat, vars, weights,30);
    REQUIRE(solver.Solve(sat));
  }
  SECTION("Max bound negative") {
    REQUIRE_THROWS(GeneralizedTotaliser::Encode(sat, vars,weights, -1));
  }
}
TEST_CASE("Generalize totaliser: Test single var", "[encoder]") {
  solver_wrappers::SimpleSolver solver;
  SatProblem sat(1);
  SECTION("No limit") {
    GeneralizedTotaliser::Encode(sat, {Lit(0)}, {1}, 1);
    SECTION("Var not set") { REQUIRE(solver.Solve(sat)); }
    SECTION("Var true") {
      sat.AddClause({Lit(0)});
      REQUIRE(solver.Solve(sat));
    }
    SECTION("Var false") {
      sat.AddClause({~Lit(0)});
      REQUIRE(solver.Solve(sat));
    }
  }
  SECTION("None") {
    GeneralizedTotaliser::Encode(sat, {Lit(0)}, {1}, 0);
    SECTION("Var not set") { REQUIRE(solver.Solve(sat)); }
    SECTION("Var true") {
      sat.AddClause({Lit(0)});
      REQUIRE_FALSE(solver.Solve(sat));
    }
    SECTION("Var false") {
      sat.AddClause({~Lit(0)});
      REQUIRE(solver.Solve(sat));
    }
  }
  SECTION("Weights") {
    SECTION("No limit") {
      GeneralizedTotaliser::Encode(sat, {Lit(0)}, {3}, 3);
      SECTION("Var not set") { REQUIRE(solver.Solve(sat)); }
      SECTION("Var true") {
        sat.AddClause({Lit(0)});
        REQUIRE(solver.Solve(sat));
      }
      SECTION("Var false") {
        sat.AddClause({~Lit(0)});
        REQUIRE(solver.Solve(sat));
      }
    }
    SECTION("None") {
      GeneralizedTotaliser::Encode(sat, {Lit(0)}, {3}, 2);
      SECTION("Var not set") { REQUIRE(solver.Solve(sat)); }
      SECTION("Var true") {
        sat.AddClause({Lit(0)});
        REQUIRE_FALSE(solver.Solve(sat));
      }
      SECTION("Var false") {
        sat.AddClause({~Lit(0)});
        REQUIRE(solver.Solve(sat));
      }
    }
  }


}
TEST_CASE("Generalize totaliser: Test multiple var", "[encoder]") {
  solver_wrappers::SimpleSolver solver;
  SatProblem sat(10);
  std::vector<Lit> vars;
  std::vector<int> weights;
  for (int i = 0; i < 10; ++i) {
    vars.push_back(Lit(i));
    weights.push_back(1);
  }
  SECTION("Nothing set") {
    SECTION("All allowed") {
      GeneralizedTotaliser::Encode(sat, vars, weights, 10);
      REQUIRE(solver.Solve(sat));
    }
    SECTION("None allowed") {
      GeneralizedTotaliser::Encode(sat, vars, weights, 0);
      REQUIRE(solver.Solve(sat));
    }
    SECTION("Some allowed") {
      GeneralizedTotaliser::Encode(sat, vars, weights, 6);
      REQUIRE(solver.Solve(sat));
    }
  }
  SECTION("Some set") {
    SECTION("False set") {
      for (int i = 0; i < 5; i++) {
        sat.AddClause({~Lit(2 * i)});
      }
        GeneralizedTotaliser::Encode(sat, vars, weights, 10);
        REQUIRE(solver.Solve(sat));
    }
    SECTION("True set") {
      for (int i = 0; i < 5; i++) {
        sat.AddClause({Lit(2 * i)});
      }
      SECTION("Possible") {
        GeneralizedTotaliser::Encode(sat, vars, weights, 5);
        REQUIRE(solver.Solve(sat));
      }
      SECTION("Max not possible") {
        GeneralizedTotaliser::Encode(sat, vars, weights, 4);
        REQUIRE_FALSE(solver.Solve(sat));
      }
    }
  }
  SECTION("Count result") {
    GeneralizedTotaliser::Encode(sat, vars, weights, 6);
    REQUIRE(solver.Solve(sat));
    std::vector<bool> sol = solver.GetSolution();
    int tot = 0;
    for (int i = 0; i < 10; ++i)
      tot += sol[i] ? 1 : 0;
    REQUIRE(tot <=6);
  }
}

TEST_CASE("Generalize totaliser: Test multiple var different weights", "[encoder]") {
  solver_wrappers::SimpleSolver solver;
  SatProblem sat(10);
  std::vector<Lit> vars;
  std::vector<int> weights;
  for (int i = 0; i < 10; ++i) {
    vars.push_back(Lit(i));
    weights.push_back(i + 1);
  }
  SECTION("None fixed") {
    int limit = 5;
    GeneralizedTotaliser::Encode(sat, vars, weights, limit);
    REQUIRE(solver.Solve(sat));
    int tot = 0;
    std::vector<bool> sol = solver.GetSolution();
    for (int i = 0; i < weights.size(); ++i) {
      if (sol[i])
        tot+=weights[i];
    }
    REQUIRE(tot <= limit);
  }
  SECTION("Some fixed") {
    sat.AddClause({~Lit(1)});
    sat.AddClause({Lit(3)});
    sat.AddClause({Lit(4)});
    sat.AddClause({~Lit(6)});
    SECTION("Not possible") {
      int limit = 8;
      GeneralizedTotaliser::Encode(sat,vars,weights, limit);
      REQUIRE_FALSE(solver.Solve(sat));
    }
    SECTION("Tight possible") {
      int limit = 9;
      GeneralizedTotaliser::Encode(sat, vars, weights, limit);
      REQUIRE(solver.Solve(sat));
      int tot = 0;
      std::vector<bool> sol = solver.GetSolution();
      for (int i = 0; i < weights.size(); ++i) {
        if (sol[i])
          tot+=weights[i];
      }
      REQUIRE(tot == limit);
    }
    SECTION("Possible") {
      int limit = 20;
      GeneralizedTotaliser::Encode(sat, vars, weights, limit);
      REQUIRE(solver.Solve(sat));
      int tot = 0;
      std::vector<bool> sol = solver.GetSolution();
      for (int i = 0; i < weights.size(); ++i) {
        if (sol[i])
          tot+=weights[i];
      }
      REQUIRE(tot <= limit);
    }

  }
}
} // namespace simple_sat_solver::sat