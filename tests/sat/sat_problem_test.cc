//
// Created by jens on 19-09-20.
//
#include "lib/catch2/catch.hpp"
#include "src/sat/sat_problem.h"
#include "src/solver_wrappers/simple_solver.h"

namespace simple_sat_solver::sat {
bool TestPartialAssignment(SatProblem s, std::vector<bool> assignments) {
  std::string info_string;
  for (int i = 0; i < assignments.size(); i++) {
    s.AddClause({Lit(i, !assignments[i])});
    info_string += assignments[i] ? "T " : "F ";
  }
  UNSCOPED_INFO(info_string);
  return solver_wrappers::SimpleSolver().Solve(s);
}
TEST_CASE("Test constructor", "[sat]") {
  SECTION("New problem") {
    SatProblem s = SatProblem(3);
    REQUIRE(s.GetNrVars() == 3);
    REQUIRE(s.GetClauses().size() == 0);
  }
  SECTION("Negative vars") {
    SatProblem s = SatProblem(-1);
    REQUIRE(s.GetNrVars() == 0);
    REQUIRE(s.GetClauses().size() == 0);
  }
}

TEST_CASE("Add clause", "[sat]") {
  SatProblem s = SatProblem(10);
  REQUIRE(s.GetClauses().size() == 0);
  SECTION("Empty clause") {
    s.AddClause({});
    REQUIRE(s.GetClauses().size() == 1);
    REQUIRE(s.GetClauses()[0] == std::vector<Lit>());
  }
  SECTION("Unit clause") {
    s.AddClause({Lit(2, false)});
    s.AddClause({Lit(5, true)});
    REQUIRE(s.GetClauses().size() == 2);
    REQUIRE(s.GetClauses()[0] == std::vector<Lit>({Lit(2, false)}));
    REQUIRE(s.GetClauses()[1] == std::vector<Lit>({Lit(5, true)}));
  }
  SECTION("Larger clause") {
    s.AddClause({Lit(4, true), Lit(2, true), Lit(1, false)});
    s.AddClause({Lit(2, false), Lit(2, true), Lit(5, false), Lit(8, true)});
    REQUIRE(s.GetClauses().size() == 2);
    REQUIRE(s.GetClauses()[0] ==
            std::vector<Lit>({Lit(4, true), Lit(2, true), Lit(1, false)}));
    REQUIRE(s.GetClauses()[1] ==
            std::vector<Lit>(
                {Lit(2, false), Lit(2, true), Lit(5, false), Lit(8, true)}));
  }
  SECTION("Min id") {
    s.AddClause({Lit(0, true)});
    REQUIRE(s.GetClauses().size() == 1);
    REQUIRE(s.GetClauses()[0] == std::vector<Lit>({Lit(0, true)}));
  }
  SECTION("Max id") {
    s.AddClause({Lit(9, true)});
    REQUIRE(s.GetClauses().size() == 1);
    REQUIRE(s.GetClauses()[0] == std::vector<Lit>({Lit(9, true)}));
  }
  SECTION("Negative id") { REQUIRE_THROWS(s.AddClause({Lit(-1)})); }
  SECTION("To large id") { REQUIRE_THROWS(s.AddClause({Lit(10)})); }
}

TEST_CASE("Test assignment", "[sat]") {
  SECTION("Wrong number of arguments") {
    SatProblem s(3);
    REQUIRE_THROWS(s.TestAssignment({true, true}));
    REQUIRE_THROWS(s.TestAssignment({true, true, true, true}));
  }
  SECTION("No clauses") {
    SatProblem s(2);
    REQUIRE(s.TestAssignment({true, false}));
  }
  SECTION("No clauses no vars") {
    SatProblem s(0);
    REQUIRE(s.TestAssignment({}));
  }
  SECTION("Empty clause") {
    SatProblem s(0);
    s.AddClause({});
    REQUIRE_FALSE(s.TestAssignment({}));
  }
  SECTION("Unit clause") {
    SatProblem s(2);
    SECTION("A") {
      s.AddClause({Lit(0, false)});
      REQUIRE(s.TestAssignment({true, false}));
      REQUIRE_FALSE(s.TestAssignment({false, false}));
    }
    SECTION("~A") {
      s.AddClause({Lit(0, true)});
      REQUIRE(s.TestAssignment({false, false}));
      REQUIRE_FALSE(s.TestAssignment({true, false}));
    }
    SECTION("B") {
      s.AddClause({Lit(1, false)});
      REQUIRE(s.TestAssignment({true, true}));
      REQUIRE_FALSE(s.TestAssignment({false, false}));
    }
    SECTION("~B") {
      s.AddClause({Lit(0, false)});
      REQUIRE(s.TestAssignment({true, false}));
      REQUIRE_FALSE(s.TestAssignment({false, true}));
    }
  }
  SECTION("Larger clause") {
    SatProblem s(5);
    s.AddClause({Lit(3), ~Lit(4), ~Lit(1), Lit(2)});
    SECTION("False") {
      REQUIRE_FALSE(s.TestAssignment({true, true, false, false, true}));
    }
    SECTION("One true assignments") {
      REQUIRE(s.TestAssignment({true, true, false, true, true}));
      REQUIRE(s.TestAssignment({true, true, false, false, false}));
      REQUIRE(s.TestAssignment({true, false, false, false, true}));
      REQUIRE(s.TestAssignment({true, true, true, false, true}));
    }
    SECTION("Multiple true assignments") {
      REQUIRE(s.TestAssignment({true, true, true, true, true}));
    }
  }
  SECTION("Multiple clauses") {
    SatProblem s(5);
    s.AddClause({Lit(3), ~Lit(4), ~Lit(1), Lit(2)});
    s.AddClause({Lit(0)});
    s.AddClause({Lit(2), ~Lit(3)});
    SECTION("All true") {
      REQUIRE(s.TestAssignment({true, false, true, true, false}));
    }
    SECTION("One false") {
      REQUIRE_FALSE(s.TestAssignment({false, false, true, false, true}));
      REQUIRE_FALSE(s.TestAssignment({true, true, false, false, true}));
      REQUIRE_FALSE(s.TestAssignment({true, false, false, true, false}));
    }
    SECTION("Multiple false") {
      REQUIRE_FALSE(s.TestAssignment({false, true, false, false, true}));
    }
  }
}
TEST_CASE("At least one", "[sat]") {
  SatProblem s(5);
  SECTION("Empty") {
    s.AtLeastOne({});
    REQUIRE_FALSE(s.TestAssignment({false, false, false, false, false}));
  }
  SECTION("Single") {
    s.AtLeastOne({Lit(0)});
    REQUIRE(s.TestAssignment({true, false, false, false, false}));
    REQUIRE_FALSE(s.TestAssignment({false, false, false, false, false}));
  }
  SECTION("Multiple") {
    s.AtLeastOne({Lit(0), Lit(1), ~Lit(2)});

    REQUIRE(s.TestAssignment({true, false, true, false, false}));
    REQUIRE(s.TestAssignment({false, true, true, false, false}));
    REQUIRE(s.TestAssignment({false, false, false, false, false}));
    REQUIRE(s.TestAssignment({true, true, true, false, false}));
    REQUIRE(s.TestAssignment({true, false, false, false, false}));
    REQUIRE(s.TestAssignment({false, true, false, false, false}));
    REQUIRE(s.TestAssignment({true, true, false, false, false}));
    REQUIRE_FALSE(s.TestAssignment({false, false, true, false, false}));
  }
}
TEST_CASE("At most one", "[sat]") {
  SatProblem s(5);
  SECTION("Empty") {
    s.AtMostOne({});
    REQUIRE(s.TestAssignment({false, false, false, false, false}));
  }
  SECTION("Single") {
    s.AtMostOne({Lit(0)});
    REQUIRE(s.TestAssignment({true, false, false, false, false}));
    REQUIRE(s.TestAssignment({false, false, false, false, false}));
  }
  SECTION("Multiple") {
    s.AtMostOne({Lit(0), Lit(1), ~Lit(2)});

    REQUIRE(s.TestAssignment({false, false, true, false, false}));
    REQUIRE(s.TestAssignment({true, false, true, false, false}));
    REQUIRE(s.TestAssignment({false, true, true, false, false}));
    REQUIRE(s.TestAssignment({false, false, false, false, false}));
    REQUIRE_FALSE(s.TestAssignment({true, true, true, false, false}));
    REQUIRE_FALSE(s.TestAssignment({true, false, false, false, false}));
    REQUIRE_FALSE(s.TestAssignment({false, true, false, false, false}));
    REQUIRE_FALSE(s.TestAssignment({true, true, false, false, false}));
  }
}
TEST_CASE("Exactly one", "[sat]") {
  SatProblem s(5);
  SECTION("Empty") {
    s.ExactlyOne({});
    REQUIRE_FALSE(s.TestAssignment({false, false, false, false, false}));
  }
  SECTION("Single") {
    s.ExactlyOne({Lit(0)});
    REQUIRE(s.TestAssignment({true, false, false, false, false}));
    REQUIRE_FALSE(s.TestAssignment({false, false, false, false, false}));
  }
  SECTION("Multiple") {
    s.ExactlyOne({Lit(0), Lit(1), ~Lit(2)});

    REQUIRE_FALSE(s.TestAssignment({false, false, true, false, false}));
    REQUIRE(s.TestAssignment({true, false, true, false, false}));
    REQUIRE(s.TestAssignment({false, true, true, false, false}));
    REQUIRE(s.TestAssignment({false, false, false, false, false}));
    REQUIRE_FALSE(s.TestAssignment({true, true, true, false, false}));
    REQUIRE_FALSE(s.TestAssignment({true, false, false, false, false}));
    REQUIRE_FALSE(s.TestAssignment({false, true, false, false, false}));
    REQUIRE_FALSE(s.TestAssignment({true, true, false, false, false}));
  }
}
TEST_CASE("Implies", "[sat]") {
  SatProblem s(2);
  SECTION("A -> B") {
    s.Implies(Lit(0), Lit(1));
    REQUIRE(s.TestAssignment({false, false}));
    REQUIRE(s.TestAssignment({false, true}));
    REQUIRE_FALSE(s.TestAssignment({true, false}));
    REQUIRE(s.TestAssignment({true, true}));
  }
  SECTION("B -> A") {
    s.Implies(Lit(1), Lit(0));
    REQUIRE(s.TestAssignment({false, false}));
    REQUIRE_FALSE(s.TestAssignment({false, true}));
    REQUIRE(s.TestAssignment({true, false}));
    REQUIRE(s.TestAssignment({true, true}));
  }
  SECTION("A -> ~B") {
    s.Implies(Lit(0), ~Lit(1));
    REQUIRE(s.TestAssignment({false, false}));
    REQUIRE(s.TestAssignment({false, true}));
    REQUIRE(s.TestAssignment({true, false}));
    REQUIRE_FALSE(s.TestAssignment({true, true}));
  }
  SECTION("~A -> B") {
    s.Implies(~Lit(0), Lit(1));
    REQUIRE_FALSE(s.TestAssignment({false, false}));
    REQUIRE(s.TestAssignment({false, true}));
    REQUIRE(s.TestAssignment({true, false}));
    REQUIRE(s.TestAssignment({true, true}));
  }

  SECTION("~A -> ~B") {
    s.Implies(~Lit(0), ~Lit(1));
    REQUIRE(s.TestAssignment({false, false}));
    REQUIRE_FALSE(s.TestAssignment({false, true}));
    REQUIRE(s.TestAssignment({true, false}));
    REQUIRE(s.TestAssignment({true, true}));
  }
}

TEST_CASE("At most k", "[sat]") {
  SatProblem s(5);
  SECTION("k=-1") {
    REQUIRE_THROWS(s.AtMostK(-1, {Lit(0)}));
  }
  SECTION("k=0") {
    int k = 0;
    SECTION("empty") {
      s.AtMostK(k, {});
      REQUIRE(s.TestAssignment({true, true, true, true, true}));
    }
    SECTION("Not empty") {
      s.AtMostK(k, {Lit(0), Lit(1), Lit(2)});
      REQUIRE(TestPartialAssignment(s,{false, false, false}));
      REQUIRE_FALSE(TestPartialAssignment(s,{true, false, false}));
      REQUIRE_FALSE(TestPartialAssignment(s,{false, true, false}));
      REQUIRE_FALSE(TestPartialAssignment(s,{true, false, true}));
      REQUIRE_FALSE(TestPartialAssignment(s,{true, true, true}));
    }
  }
  SECTION("K=1") {
    int k = 1;
    SECTION("Empty") {
      s.AtMostK(k, {});
      REQUIRE(s.TestAssignment({false, false, false, false, false}));
    }
    SECTION("Single") {
      s.AtMostK(k, {Lit(0)});
      REQUIRE(s.TestAssignment({true, false, false, false, false}));
      REQUIRE(s.TestAssignment({false, false, false, false, false}));
    }
    SECTION("Multiple") {
      s.AtMostK(k, {Lit(0), Lit(1), ~Lit(2)});
      SECTION("Passed assignments") {
        SECTION("Total 0") {
          REQUIRE(TestPartialAssignment(s, {false, false, true}));
        }
        SECTION("Total 1") {
          REQUIRE(TestPartialAssignment(s, {true, false, true}));
          REQUIRE(TestPartialAssignment(s, {false, true, true}));
          REQUIRE(TestPartialAssignment(s, {false, false, false}));
        }
      }
      SECTION("Failed assignments") {
        SECTION("Total 2") {
          REQUIRE_FALSE(TestPartialAssignment(s, {true, true, true}));
          REQUIRE_FALSE(TestPartialAssignment(s, {true, false, false}));
          REQUIRE_FALSE(TestPartialAssignment(s, {false, true, false}));
        }
        SECTION("Total 3 ") {
          REQUIRE_FALSE(TestPartialAssignment(s, {true, true, false}));
        }
      }
    }
  }
  SECTION("K = 3") {
    int k = 3;
    SECTION("Empty") {
      s.AtMostK(k, {});
      REQUIRE(s.TestAssignment({false, false, false, true, true}));
    }
    SECTION("Double") {
      s.AtMostK(k, {Lit(0), Lit(1)});
      REQUIRE(TestPartialAssignment(s, {false, false}));
      REQUIRE(TestPartialAssignment(s, {false, true}));
      REQUIRE(TestPartialAssignment(s, {true, false}));
      REQUIRE(TestPartialAssignment(s, {true, true}));
    }
    SECTION("Triple") {
      s.AtMostK(k, {Lit(0), Lit(1), ~Lit(2)});
      REQUIRE(TestPartialAssignment(s, {true, true, false}));
    }
    SECTION("5 lits") {
      s.AtMostK(k, {Lit(0), Lit(1), Lit(2), ~Lit(3), ~Lit(4)});
      SECTION("less than k") {
        REQUIRE(TestPartialAssignment(s, {false, false, false, true, true}));
        REQUIRE(TestPartialAssignment(s, {false, true, false, true, true}));
        REQUIRE(TestPartialAssignment(s, {false, true, false, false, true}));
      }
      SECTION("Exactly k") {
        REQUIRE(TestPartialAssignment(s, {true, true, true, true, true}));
        REQUIRE(TestPartialAssignment(s, {false, false, true, false, false}));
        REQUIRE(TestPartialAssignment(s, {false, true, false, false, false}));
        REQUIRE(TestPartialAssignment(s, {true, true, false, false, true}));
      }
      SECTION("More than k") {
        REQUIRE_FALSE(
            TestPartialAssignment(s, {false, true, true, false, false}));
        REQUIRE_FALSE(
            TestPartialAssignment(s, {true, false, true, false, false}));
        REQUIRE_FALSE(
            TestPartialAssignment(s, {true, true, false, false, false}));
        REQUIRE_FALSE(
            TestPartialAssignment(s, {true, true, true, true, false}));
        REQUIRE_FALSE(
            TestPartialAssignment(s, {true, true, true, false, true}));
        REQUIRE_FALSE(
            TestPartialAssignment(s, {true, true, true, false, false}));
      }
    }
  }
}

} // namespace simple_sat_solver::sat