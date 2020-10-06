//
// Created by jens on 05-10-20.
//

#include "lib/catch2/catch.hpp"
#include "src/sat/encoders/cardinality_network.h"
#include "src/solver_wrappers/simple_solver.h"

namespace simple_sat_solver::sat {
bool CheckBound(std::vector<bool> sol, int vars, int max) {
    int tot = 0;
    for (int i = 0; i < vars; ++i) {
      tot += sol[i] ? 1 : 0;
    }
    return tot <= max;
}
TEST_CASE("CardinalityNetwork: Min max bounds", "[encoder]") {
  solver_wrappers::SimpleSolver solver;
  SatProblem sat(3);
  std::vector<Lit> vars = {Lit(0), Lit(1), Lit(2)};
  SECTION("Max tight bounds") {
    CardinalityNetwork::Encode(sat, vars, 3);
    REQUIRE(solver.Solve(sat));
    REQUIRE(CheckBound(solver.GetSolution(), vars.size(), 3));
  }

  SECTION("Large max bound") {
    CardinalityNetwork::Encode(sat, vars, 30);
    REQUIRE(solver.Solve(sat));
    REQUIRE(CheckBound(solver.GetSolution(), vars.size(), 30));
  }
  SECTION("Max bound negative") {
    REQUIRE_THROWS(CardinalityNetwork::Encode(sat, vars, -1));
  }
}
TEST_CASE("CardinalityNetwork: Test single var", "[encoder]") {
  solver_wrappers::SimpleSolver solver;
  SatProblem sat(1);
  SECTION("No limit") {
    CardinalityNetwork::Encode(sat, {Lit(0)}, 1);
    SECTION("Var not set") { REQUIRE(solver.Solve(sat)); }
    SECTION("Var true") {
      sat.AddClause({Lit(0)});
      REQUIRE(solver.Solve(sat));
      REQUIRE(CheckBound(solver.GetSolution(), 1, 1));
    }
    SECTION("Var false") {
      sat.AddClause({~Lit(0)});
      REQUIRE(solver.Solve(sat));
      REQUIRE(CheckBound(solver.GetSolution(), 1, 1));
    }
  }

  SECTION("None") {
    CardinalityNetwork::Encode(sat, {Lit(0)}, 0);
    SECTION("Var not set") { REQUIRE(solver.Solve(sat)); }
    SECTION("Var true") {
      sat.AddClause({Lit(0)});
      REQUIRE_FALSE(solver.Solve(sat));
    }
    SECTION("Var false") {
      sat.AddClause({~Lit(0)});
      REQUIRE(solver.Solve(sat));
      REQUIRE(CheckBound(solver.GetSolution(), 1, 0));
    }
  }
}
TEST_CASE("CardinalityNetwork: 2 vars", "[encoder]") {
  solver_wrappers::SimpleSolver solver;
  SatProblem sat(2);
  std::vector<Lit> vars;
  for (int i = 0; i < 2; ++i) {
    vars.push_back(Lit(i));
  }

  SECTION("Nothing set") {
    CardinalityNetwork::Encode(sat, vars, 1);
    REQUIRE(solver.Solve(sat));
    REQUIRE(CheckBound(solver.GetSolution(), vars.size(), 1));
  }
  SECTION("Set true, possible") {
    sat.AddClause({Lit(0)});
    CardinalityNetwork::Encode(sat, vars, 1);
    REQUIRE(solver.Solve(sat));
    std::vector<bool> sol;
    sol = solver.GetSolution();
    REQUIRE(sol[0] == true);
    REQUIRE(sol[1] == false);
  }
  SECTION("Set false, possible") {
    sat.AddClause({~Lit(0)});
    CardinalityNetwork::Encode(sat, vars, 1);
    REQUIRE(solver.Solve(sat));
    std::vector<bool> sol;
    sol = solver.GetSolution();
    REQUIRE(sol[0] == false);
  }
  SECTION("Set true, not possible") {
    sat.AddClause({Lit(0)});
    sat.AddClause({Lit(1)});
    CardinalityNetwork::Encode(sat, vars, 1);
    REQUIRE_FALSE(solver.Solve(sat));
  }
}
TEST_CASE("CardinalityNetwork: Test multiple var", "[encoder]") {
  solver_wrappers::SimpleSolver solver;
  SatProblem sat(10);
  std::vector<Lit> vars;
  for (int i = 0; i < 10; ++i) {
    vars.push_back(Lit(i));
  }
  SECTION("Nothing set") {
    SECTION("All allowed") {
      CardinalityNetwork::Encode(sat, vars, 10);
      REQUIRE(solver.Solve(sat));
    }
    SECTION("None allowed") {
      CardinalityNetwork::Encode(sat, vars, 0);
      REQUIRE(solver.Solve(sat));
      REQUIRE(CheckBound(solver.GetSolution(), vars.size(), 0));
    }

    SECTION("Some") {
      CardinalityNetwork::Encode(sat, vars, 8);
      REQUIRE(solver.Solve(sat));
      REQUIRE(CheckBound(solver.GetSolution(), vars.size(), 8));
    }
  }
  SECTION("Some set") {
    SECTION("False set") {
      for (int i = 0; i < 5; i++) {
        sat.AddClause({~Lit(2 * i)});
      }
      SECTION("Possible") {
        CardinalityNetwork::Encode(sat, vars, 10);
        REQUIRE(solver.Solve(sat));
        REQUIRE(CheckBound(solver.GetSolution(), vars.size(), 10));
      }
      SECTION("Also possible") {
        CardinalityNetwork::Encode(sat, vars, 8);
        REQUIRE(solver.Solve(sat));
        REQUIRE(CheckBound(solver.GetSolution(), vars.size(), 8));
      }
    }
    SECTION("True set") {
      sat.AddClause({Lit(0)});
      sat.AddClause({Lit(2)});
      sat.AddClause({Lit(4)});
      sat.AddClause({Lit(7)});
      sat.AddClause({Lit(5)});
      SECTION("Possible 6") {
        CardinalityNetwork n = CardinalityNetwork::Encode(sat, vars, 6);
        REQUIRE(solver.Solve(sat));
        REQUIRE(CheckBound(solver.GetSolution(), vars.size(), 6));
      }
      SECTION("Possible 8") {
        CardinalityNetwork::Encode(sat, vars, 8);
        REQUIRE(solver.Solve(sat));
        REQUIRE(CheckBound(solver.GetSolution(), vars.size(), 8));
      }
    SECTION("Tight possible"){
        CardinalityNetwork::Encode(sat, vars, 5);
        REQUIRE(solver.Solve(sat));
        REQUIRE(CheckBound(solver.GetSolution(), vars.size(), 5));
    }
      SECTION("Max not possible") {
        CardinalityNetwork::Encode(sat, vars, 4);
        REQUIRE_FALSE(solver.Solve(sat));
      }
    }
  }
}
} 