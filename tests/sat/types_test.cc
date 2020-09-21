//
// Created by jens on 18-09-20.
//
#include "lib/catch2/catch.hpp"
#include "src/sat/types.h"
namespace simple_sat_solver::sat {
TEST_CASE("The Lit constructor", "[Lit]") {
  SECTION("Test complete constructor") {
    Lit l(2, true);
    REQUIRE(l.x == 2);
    REQUIRE(l.complement == true);

    Lit l2(3, false);

    REQUIRE(l2.x == 3);
    REQUIRE(!l2.complement);
  }
  SECTION("Var constructor") {
    Lit l(5);
    REQUIRE(l.x == 5);
    REQUIRE(l.complement == false);
  }
}

TEST_CASE("Lit operators", "[Lit]") {
  SECTION("Equal lit numbers") {
    Lit l1(5, true);
    Lit l2(5, true);

    Lit l3(2, false);
    Lit l4(2, false);

    SECTION("Test ==") {
      REQUIRE(l1 == l2);
      REQUIRE(l3 == l4);
    }

    SECTION("Test !=") {
      REQUIRE_FALSE(l1 != l2);
      REQUIRE_FALSE(l3 != l4);
    }
  }

  SECTION("Test different") {
    Lit l1(2, true);
    Lit l2(3, true);
    Lit l3(2, false);
    Lit l4(3, false);
    SECTION("Test ==") {
      REQUIRE_FALSE(l1 == l2);
      REQUIRE_FALSE(l1 == l3);
      REQUIRE_FALSE(l1 == l4);
      REQUIRE_FALSE(l2 == l3);
      REQUIRE_FALSE(l2 == l4);
      REQUIRE_FALSE(l3 == l4);
    }
    SECTION("Test !=") {
      REQUIRE(l1 != l2);
      REQUIRE(l1 != l3);
      REQUIRE(l1 != l4);
      REQUIRE(l2 != l3);
      REQUIRE(l2 != l4);
      REQUIRE(l3 != l4);
    }
  }
  SECTION("Operator ~") {
    SECTION("normal to complement") {
      Lit l1(2, false);
      Lit l2 = ~l1;

      REQUIRE(l1.complement == false);
      REQUIRE(l2.complement == true);
      REQUIRE(l2.x == 2);

      Lit l3 = ~Lit(5);
      REQUIRE(l3.complement == true);
    }
    SECTION("complement to normal") {
      Lit l1(2, true);
      Lit l2 = ~l1;

      REQUIRE(l1.complement == true);
      REQUIRE(l2.complement == false);
      REQUIRE(l2.x== 2);
    }

    SECTION("Differs") {
      Lit l(4);
      REQUIRE_FALSE(l == ~l);
    }
    SECTION("~~ Same") {
      Lit l(2);
      REQUIRE(l == ~~l);
    }
  }
}
} // namespace simple_sat_solver::sat