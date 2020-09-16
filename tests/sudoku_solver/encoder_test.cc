//
// Created by jens on 16-09-20.
//
#include <algorithm>

#include "lib/catch.hpp"
#include "src/sudoku_solver/encoder.h"
namespace simple_sat_solver::sudoku {

TEST_CASE("Test 2x2 structure", "[sudoku]") {
  SatProblem *p = Encoder::Encode(2, std::vector<int>());
  SECTION("Size") { REQUIRE(p->nr_vars == 4); }

  SECTION("All lits uses existing vars") {
    for (const auto& c : p->clauses) {
      for(auto l : c) {
        REQUIRE(l.x >= 0);
        REQUIRE(l.x < 4 * 4 * 4);
      }
    }
  }

  SECTION("At least 1 cell") {
    std::vector<solver::Lit> at_least_1;
    int startIndex = 0;
    SECTION("Cell x=0, y=0") { startIndex = 0; }
    SECTION("Cell x=2, y=3") { startIndex = (2 + 3 * 4) * 4; }
    SECTION("Cell x=3, y=3") { startIndex = 4 * 3 * 4; }
    at_least_1.emplace_back(0 + startIndex, false);
    at_least_1.emplace_back(1 + startIndex, false);
    at_least_1.emplace_back(2 + startIndex, false);
    at_least_1.emplace_back(3 + startIndex, false);
    REQUIRE(std::find(p->clauses.begin(), p->clauses.end(), at_least_1) !=
        p->clauses.end());
  }
  SECTION("At least 1 cell constraint for wrong startIndex") {
    std::vector<solver::Lit> at_least_1;
    int startIndex = 1;
    at_least_1.emplace_back(0 + startIndex, false);
    at_least_1.emplace_back(1 + startIndex, false);
    at_least_1.emplace_back(2 + startIndex, false);
    at_least_1.emplace_back(3 + startIndex, false);
    REQUIRE(std::find(p->clauses.begin(), p->clauses.end(), at_least_1) ==
            p->clauses.end());
  }
  SECTION("At most 1 cell") {
    int startIndex = 0;
    SECTION("Cell x=0, y=0") { startIndex = 0; }
    SECTION("Cell x=2, y=3") { startIndex = (2 + 3 * 4) * 4; }
    SECTION("Cell x=3, y=3") { startIndex = 4 * 3 * 4; }
    std::vector<solver::Lit> at_most_1;
    for (int x = 0; x < 4; x++) {
      for (int y = 0; y < 4; y++) {
        if (x == y)
          continue;
        int f = std::min(x, y) + startIndex;
        int s = std::max(x, y) + startIndex;
        std::vector<solver::Lit> clause;
        clause.emplace_back(f, true);
        clause.emplace_back(s, true);
        REQUIRE(std::find(p->clauses.begin(), p->clauses.end(), clause) !=
                p->clauses.end());
      }
    }
  }
}
} // namespace simple_sat_solver::sudoku