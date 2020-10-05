//
// Created by jens on 05-10-20.
//

#include "lib/catch2/catch.hpp"
#include "src/sat/encoders/totaliser_encoder.h"
#include "src/solver_wrappers/simple_solver.h"

namespace simple_sat_solver::sat {
TEST_CASE("Totaliser: Min max bounds", "[encoder]") {
  solver_wrappers::SimpleSolver solver;
  SatProblem sat(3);
  std::vector<Lit> vars = {Lit(0), Lit(1), Lit(2)};
  SECTION("Max tight bounds") {
    TotaliserEncoder::Encode(sat, vars, 0, 3);
    REQUIRE(solver.Solve(sat));
  }
  SECTION("Negative min bound") {
    TotaliserEncoder::Encode(sat, vars, -10, 3);
    REQUIRE(solver.Solve(sat));
  }
  SECTION("Large max bound") {
    TotaliserEncoder::Encode(sat, vars, 0, 30);
    REQUIRE(solver.Solve(sat));
  }
  SECTION("Max bound negative") {
    REQUIRE_THROWS(TotaliserEncoder::Encode(sat, vars, -2, -1));
  }
  SECTION("Min bound large") {
    TotaliserEncoder::Encode(sat, vars, 5, 10);
    REQUIRE_FALSE(solver.Solve(sat));
  }
}
TEST_CASE("Totaliser: Test single var", "[encoder]") {
  solver_wrappers::SimpleSolver solver;
  SatProblem sat(1);
  SECTION("No limit") {
    TotaliserEncoder::Encode(sat, {Lit(0)}, 0, 1);
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
  SECTION("At least 1") {
    TotaliserEncoder::Encode(sat, {Lit(0)}, 1, 1);
    SECTION("Var not set") { REQUIRE(solver.Solve(sat)); }
    SECTION("Var true") {
      sat.AddClause({Lit(0)});
      REQUIRE(solver.Solve(sat));
    }
    SECTION("Var false") {
      sat.AddClause({~Lit(0)});
      REQUIRE_FALSE(solver.Solve(sat));
    }
  }
  SECTION("None") {
    TotaliserEncoder::Encode(sat, {Lit(0)}, 0, 0);
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
TEST_CASE("Totaliser: Test multiple var", "[encoder]") {
  solver_wrappers::SimpleSolver solver;
  SatProblem sat(10);
  std::vector<Lit> vars;
  for (int i = 0; i < 10; ++i) {
    vars.push_back(Lit(i));
  }
  SECTION("Nothing set") {
    SECTION("All allowed") {
      TotaliserEncoder::Encode(sat, vars, 0, 10);
      REQUIRE(solver.Solve(sat));
    }
    SECTION("None allowed") {
      TotaliserEncoder::Encode(sat, vars, 0, 0);
      REQUIRE(solver.Solve(sat));
    }
    SECTION("All required") {
      TotaliserEncoder::Encode(sat, vars, 10, 10);
      REQUIRE(solver.Solve(sat));
    }
  SECTION("Some required") {
      TotaliserEncoder::Encode(sat, vars, 4, 8);
      REQUIRE(solver.Solve(sat));
    }
  }
  SECTION("Some set") {
    SECTION("False set") {
      for (int i = 0; i < 5; i++) {
        sat.AddClause({~Lit(2 * i)});
      }
      SECTION("Possible") {
        TotaliserEncoder::Encode(sat, vars, 5, 10);
        REQUIRE(solver.Solve(sat));
        std::vector<bool> sol = solver.GetSolution();
        for (int i = 0; i < 10; ++i)
          REQUIRE(sol[i] == i %2);
      }
      SECTION("Min not possible") {
        TotaliserEncoder::Encode(sat, vars, 6, 10);
        REQUIRE_FALSE(solver.Solve(sat));
      }
    }
    SECTION("True set") {
      for (int i = 0; i < 5; i++) {
        sat.AddClause({Lit(2 * i)});
      }
      SECTION("Possible") {
        TotaliserEncoder::Encode(sat, vars, 0, 5);
        REQUIRE(solver.Solve(sat));
        std::vector<bool> sol = solver.GetSolution();
        for (int i = 0; i < 10; ++i)
          REQUIRE(sol[i] != i %2);
      }
      SECTION("Max not possible") {
        TotaliserEncoder::Encode(sat, vars, 0, 4);
        REQUIRE_FALSE(solver.Solve(sat));
      }
    }
  }
  SECTION("Exact") {
    TotaliserEncoder::Encode(sat, vars, 6, 6);
    REQUIRE(solver.Solve(sat));
    std::vector<bool> sol = solver.GetSolution();
    int tot = 0;
    for (int i = 0; i < 10; ++i)
      tot += sol[i] ? 1 : 0;
    REQUIRE(tot ==6);
  }
}
} // namespace simple_sat_solver::sat