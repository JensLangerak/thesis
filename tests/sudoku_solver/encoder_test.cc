//
// Created by jens on 16-09-20.
//
#include <algorithm>

#include "lib/catch.hpp"
#include "src/sudoku_solver/encoder.h"
namespace simple_sat_solver::sudoku {
bool IsFalse(std::vector<solver::Lit> &clause,
             std::vector<solver::LBool> &values) {
  for (auto l : clause) {
    solver::LBool value = values[l.x];
    if (value == solver::LBool::kUnknown ||
        value == solver::LBool::kTrue && (!l.complement) ||
        value == solver::LBool::kFalse && l.complement)
      return false;
  }
  return true;
}

bool IsTrue(std::vector<solver::Lit> &clause,
            std::vector<solver::LBool> &values) {
  for (auto l : clause) {
    solver::LBool value = values[l.x];
    if (value == solver::LBool::kTrue && (!l.complement) ||
        value == solver::LBool::kFalse && l.complement)
      return true;
  }
  return false;
}

bool AnyTrue(std::vector<std::vector<solver::Lit>> &clauses,
             std::vector<solver::LBool> &values) {
  for (auto c : clauses) {
    if (IsTrue(c, values))
      return true;
  }
  return false;
}
bool AnyFalse(std::vector<std::vector<solver::Lit>> &clauses,
              std::vector<solver::LBool> &values) {
  for (auto c : clauses) {
    if (IsFalse(c, values))
      return true;
  }
  return false;
}
bool AllTrue(std::vector<std::vector<solver::Lit>> &clauses,
             std::vector<solver::LBool> &values) {
  for (auto c : clauses) {
    if (!IsTrue(c, values))
      return false;
  }
  return true;
}

std::vector<solver::LBool> GetAssignments(std::vector<int> sudoku,
                                          int sub_size) {
  int size = sub_size * sub_size;
  std::vector<solver::LBool> assignments;
  assignments.reserve(size * size * size);
  for (int c : sudoku) {
    for (int v = 1; v <= size; v++) {
      if (c < 1)
        assignments.push_back(solver::LBool::kUnknown);
      else if (c == v)
        assignments.push_back(solver::LBool::kTrue);
      else
        assignments.push_back(solver::LBool::kFalse);
    }
  }
  return assignments;
}

TEST_CASE("Test helper Assignments", "[sudoku]") {
  SECTION("Sizes") {
    SECTION("2 (4x4)") {
      std::vector<int> sudoku{// @formatter:off
          0,0,0,0,
          0,0,0,0,
          0,0,0,0,
          0,0,0,0};
      // @formatter:on
      REQUIRE(GetAssignments(sudoku, 2).size() == 4 * 4 * 4);
    }
    SECTION("3 (9x9)") {
      std::vector<int> sudoku{// @formatter:off
          0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0};
      // @formatter:on
      REQUIRE(GetAssignments(sudoku, 3).size() == 9 * 9 * 9);
    }
  }
  SECTION("Assigned values") {
    std::vector<int> sudoku{// @formatter:off
        -1,0,0,0,
        0,2,0,0,
        0,0,0,0,
        0,0,4,1};
    // @formatter:on
    auto assignments = GetAssignments(sudoku, 2);
    SECTION("-1") {
      for (int i = 0; i < 4; i++) {
        REQUIRE(assignments[i] == solver::LBool::kUnknown);
      }
    }
    SECTION("0") {
      for (int i = 1 * 4; i < 1 * 4 + 4; i++) {
        REQUIRE(assignments[i] == solver::LBool::kUnknown);
      }
    }
    SECTION("2") {
      for (int i = 5 * 4; i < 5 * 4 + 4; i++) {
        if (i == 5 * 4 + 1)
          REQUIRE(assignments[i] == solver::LBool::kTrue);
        else
          REQUIRE(assignments[i] == solver::LBool::kFalse);
      }
    }
    SECTION("4") {
      for (int i = 14 * 4; i < 14 * 4 + 4; i++) {
        if (i == 14 * 4 + 3)
          REQUIRE(assignments[i] == solver::LBool::kTrue);
        else
          REQUIRE(assignments[i] == solver::LBool::kFalse);
      }
    }
    SECTION("1") {
      for (int i = 15 * 4; i < 15 * 4 + 4; i++) {
        if (i == 15 * 4 + 0)
          REQUIRE(assignments[i] == solver::LBool::kTrue);
        else
          REQUIRE(assignments[i] == solver::LBool::kFalse);
      }
    }
  }
}

TEST_CASE("Test helper Any True False methods", "[sudoku]") {
  std::vector<std::vector<solver::Lit>> clauses;
  SECTION("Empty clauses") {
    std::vector<solver::LBool> values;
    REQUIRE(AnyTrue(clauses, values) == false);
    REQUIRE(AnyFalse(clauses, values) == false);
  }
  SECTION("Multiple clauses") {
    std::vector<solver::Lit> c1;
    std::vector<solver::Lit> c2;
    std::vector<solver::Lit> c3;

    c1.emplace_back(0, false);
    c2.emplace_back(1, false);
    c3.emplace_back(2, false);

    clauses.push_back(c1);
    clauses.push_back(c2);
    clauses.push_back(c3);
    SECTION("UUU") {
      std::vector<solver::LBool> values{solver::LBool::kUnknown,
                                        solver::LBool::kUnknown,
                                        solver::LBool::kUnknown};
      SECTION("AnyTrue") { REQUIRE(AnyTrue(clauses, values) == false); }
      SECTION("AnyFalse") { REQUIRE(AnyFalse(clauses, values) == false); }
      SECTION("AllTrue") { REQUIRE(AllTrue(clauses, values) == false); }
    }
    SECTION("TUF") {
      std::vector<solver::LBool> values{
          solver::LBool::kTrue, solver::LBool::kUnknown, solver::LBool::kFalse};
      SECTION("AnyTrue") { REQUIRE(AnyTrue(clauses, values) == true); }
      SECTION("AnyFalse") { REQUIRE(AnyFalse(clauses, values) == true); }
      SECTION("AllTrue") { REQUIRE(AllTrue(clauses, values) == false); }
    }
    SECTION("UUT") {
      std::vector<solver::LBool> values{solver::LBool::kUnknown,
                                        solver::LBool::kUnknown,
                                        solver::LBool::kTrue};
      SECTION("AnyTrue") { REQUIRE(AnyTrue(clauses, values) == true); }
      SECTION("AnyFalse") { REQUIRE(AnyFalse(clauses, values) == false); }
      SECTION("AllTrue") { REQUIRE(AllTrue(clauses, values) == false); }
    }
    SECTION("FUU") {
      std::vector<solver::LBool> values{solver::LBool::kFalse,
                                        solver::LBool::kUnknown,
                                        solver::LBool::kUnknown};
      SECTION("AnyTrue") { REQUIRE(AnyTrue(clauses, values) == false); }
      SECTION("AnyFalse") { REQUIRE(AnyFalse(clauses, values) == true); }
      SECTION("AllTrue") { REQUIRE(AllTrue(clauses, values) == false); }
    }
    SECTION("TTT") {
      std::vector<solver::LBool> values{
          solver::LBool::kTrue, solver::LBool::kTrue, solver::LBool::kTrue};
      SECTION("AnyTrue") { REQUIRE(AnyTrue(clauses, values) == true); }
      SECTION("AnyFalse") { REQUIRE(AnyFalse(clauses, values) == false); }
      SECTION("AllTrue") { REQUIRE(AllTrue(clauses, values) == true); }
    }
  }
}
TEST_CASE("Test helper IsTrue IsFalse methods", "[sudoku]") {
  std::vector<solver::LBool> values;
  values.push_back(solver::LBool::kUnknown);
  values.push_back(solver::LBool::kTrue);
  values.push_back(solver::LBool::kFalse);

  std::vector<solver::Lit> c;
  SECTION("Empty clause") {
    SECTION("IsFalse") { REQUIRE(IsFalse(c, values) == true); }
    SECTION("IsTrue") { REQUIRE(IsTrue(c, values) == false); }
  }
  SECTION("Unit normal") {
    SECTION("Unknown") {
      c.emplace_back(0, false);
      SECTION("IsFalse") { REQUIRE(IsFalse(c, values) == false); }
      SECTION("IsTrue") { REQUIRE(IsTrue(c, values) == false); }
    }
    SECTION("True") {
      c.emplace_back(1, false);
      SECTION("IsFalse") { REQUIRE(IsFalse(c, values) == false); }
      SECTION("IsTrue") { REQUIRE(IsTrue(c, values) == true); }
    }
    SECTION("False") {
      c.emplace_back(2, false);
      SECTION("IsFalse") { REQUIRE(IsFalse(c, values) == true); }
      SECTION("IsTrue") { REQUIRE(IsTrue(c, values) == false); }
    }
  }
  SECTION("Unit complement") {
    SECTION("Unknown") {
      c.emplace_back(0, true);
      SECTION("IsFalse") { REQUIRE(IsFalse(c, values) == false); }
      SECTION("IsTrue") { REQUIRE(IsTrue(c, values) == false); }
    }
    SECTION("True") {
      c.emplace_back(1, true);
      SECTION("IsFalse") { REQUIRE(IsFalse(c, values) == true); }
      SECTION("IsTrue") { REQUIRE(IsTrue(c, values) == false); }
    }
    SECTION("False") {
      c.emplace_back(2, true);
      SECTION("IsFalse") { REQUIRE(IsFalse(c, values) == false); }
      SECTION("IsTrue") { REQUIRE(IsTrue(c, values) == true); }
    }
  }
  SECTION("Bigger Clause") {
    SECTION("FFF") {
      c.emplace_back(2, false);
      c.emplace_back(1, true);
      c.emplace_back(2, false);
      SECTION("IsFalse") { REQUIRE(IsFalse(c, values) == true); }
      SECTION("IsTrue") { REQUIRE(IsTrue(c, values) == false); }
    }
    SECTION("FFU") {
      c.emplace_back(2, false);
      c.emplace_back(1, true);
      c.emplace_back(0, false);
      SECTION("IsFalse") { REQUIRE(IsFalse(c, values) == false); }
      SECTION("IsTrue") { REQUIRE(IsTrue(c, values) == false); }
    }
    SECTION("FTF") {
      c.emplace_back(2, false);
      c.emplace_back(1, false);
      c.emplace_back(2, false);
      SECTION("IsFalse") { REQUIRE(IsFalse(c, values) == false); }
      SECTION("IsTrue") { REQUIRE(IsTrue(c, values) == true); }
    }
    SECTION("UFT") {
      c.emplace_back(0, false);
      c.emplace_back(1, true);
      c.emplace_back(2, true);
      SECTION("IsFalse") { REQUIRE(IsFalse(c, values) == false); }
      SECTION("IsTrue") { REQUIRE(IsTrue(c, values) == true); }
    }
    SECTION("UUU") {
      c.emplace_back(0, false);
      c.emplace_back(0, false);
      c.emplace_back(0, true);
      SECTION("IsFalse") { REQUIRE(IsFalse(c, values) == false); }
      SECTION("IsTrue") { REQUIRE(IsTrue(c, values) == false); }
    }
    SECTION("FUU") {
      c.emplace_back(2, false);
      c.emplace_back(0, false);
      c.emplace_back(0, false);
      SECTION("IsFalse") { REQUIRE(IsFalse(c, values) == false); }
      SECTION("IsTrue") { REQUIRE(IsTrue(c, values) == false); }
    }
    SECTION("TUU") {
      c.emplace_back(2, true);
      c.emplace_back(0, false);
      c.emplace_back(0, false);
      SECTION("IsFalse") { REQUIRE(IsFalse(c, values) == false); }
      SECTION("IsTrue") { REQUIRE(IsTrue(c, values) == true); }
    }
  }
}

TEST_CASE("Test 2x2 structure", "[sudoku]") {
  std::vector<int> sudoku{// @formatter:off
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0};
  // @formatter:on
  SatProblem *p = Encoder::Encode(2, sudoku);
  auto assignments = GetAssignments(sudoku, 2);
  SECTION("Size") { REQUIRE(p->nr_vars == 4); }

  SECTION("All lits uses existing vars") {
    for (const auto &c : p->clauses) {
      for (auto l : c) {
        REQUIRE(l.x >= 0);
        REQUIRE(l.x < 4 * 4 * 4);
      }
    }
  }

  SECTION("Currently solvable") {
    REQUIRE(AnyFalse(p->clauses, assignments) == false);
  }
}
TEST_CASE("Test 2x2 illegal states", "[sudoku]") {
  SECTION("Duplicated filled in cell") {
    std::vector<int> sudoku{// @formatter:off
        2, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0};
    // @formatter:on
    SatProblem *p = Encoder::Encode(2, sudoku);
    auto assignments = GetAssignments(sudoku, 2);
    assignments[0] = solver::LBool::kTrue;
    REQUIRE(AnyFalse(p->clauses, assignments));
  }
  SECTION("Duplicated in column") {
    std::vector<int> sudoku{// @formatter:off
        2, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        2, 0, 0, 0};
    // @formatter:on
    SatProblem *p = Encoder::Encode(2, sudoku);
    auto assignments = GetAssignments(sudoku, 2);
    REQUIRE(AnyFalse(p->clauses, assignments));
  }
  SECTION("Duplicated in row") {
    std::vector<int> sudoku{// @formatter:off
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 3, 0, 3,
        0, 0, 0, 0};
    // @formatter:on
    SatProblem *p = Encoder::Encode(2, sudoku);
    auto assignments = GetAssignments(sudoku, 2);
    REQUIRE(AnyFalse(p->clauses, assignments));
  }
  SECTION("Duplicated in subgrid") {
    std::vector<int> sudoku{// @formatter:off
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 4, 0,
        0, 0, 0, 4};
    // @formatter:on
    SatProblem *p = Encoder::Encode(2, sudoku);
    auto assignments = GetAssignments(sudoku, 2);
    REQUIRE(AnyFalse(p->clauses, assignments));
  }
}

TEST_CASE("Test 2x2 Legal sudoku state", "[sudoku]") {
  SECTION("Partly solved") {
    std::vector<int> sudoku{// @formatter:off
        2, 0, 0, 0,
        4, 0, 2, 0,
        0, 3, 0, 1,
        1, 0, 4, 0};
    // @formatter:on
    SatProblem *p = Encoder::Encode(2, sudoku);
    auto assignments = GetAssignments(sudoku, 2);
    REQUIRE(!AnyFalse(p->clauses, assignments));
    REQUIRE(!AllTrue(p->clauses, assignments));
  }
  SECTION("Almost solved") {
    std::vector<int> sudoku{// @formatter:off
        1, 3, 4, 2,
        2, 4, 1, 3,
        3, 1, -1, 4,
        4, 2, 3, 1};
    // @formatter:on
    SatProblem *p = Encoder::Encode(2, sudoku);
    auto assignments = GetAssignments(sudoku, 2);
    REQUIRE(!AnyFalse(p->clauses, assignments));
    REQUIRE(!AllTrue(p->clauses, assignments));
  }
  SECTION("Completely solved") {
    std::vector<int> sudoku{// @formatter:off
        1, 3, 4, 2,
        2, 4, 1, 3,
        3, 1, 2, 4,
        4, 2, 3, 1};
    // @formatter:on
    SatProblem *p = Encoder::Encode(2, sudoku);
    auto assignments = GetAssignments(sudoku, 2);
    REQUIRE(AnyFalse(p->clauses, assignments) == false);
    REQUIRE(AllTrue(p->clauses, assignments));
  }
}


TEST_CASE("Test 3x3 structure", "[sudoku]") {
  std::vector<int> sudoku{// @formatter:off
      0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0};
  // @formatter:on
  SatProblem *p = Encoder::Encode(3, sudoku);
  auto assignments = GetAssignments(sudoku, 3);
  SECTION("Size") { REQUIRE(p->nr_vars == 9); }

  SECTION("All lits uses existing vars") {
    for (const auto &c : p->clauses) {
      for (auto l : c) {
        REQUIRE(l.x >= 0);
        REQUIRE(l.x < 9 * 9 * 9);
      }
    }
  }

  SECTION("Currently solvable") {
    REQUIRE(AnyFalse(p->clauses, assignments) == false);
  }
}
TEST_CASE("Test 3x3 illegal states", "[sudoku]") {
  SECTION("Duplicated filled in cell") {

    std::vector<int> sudoku{// @formatter:off
        2,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0};
    // @formatter:on
    SatProblem *p = Encoder::Encode(3, sudoku);
    auto assignments = GetAssignments(sudoku, 3);
    assignments[0] = solver::LBool::kTrue;
    REQUIRE(AnyFalse(p->clauses, assignments));
  }
  SECTION("Duplicated in column") {

    std::vector<int> sudoku{// @formatter:off
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,1,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,1,0,0,0,0,0};
    // @formatter:on
    SatProblem *p = Encoder::Encode(3, sudoku);
    auto assignments = GetAssignments(sudoku, 3);
    REQUIRE(AnyFalse(p->clauses, assignments));
  }
  SECTION("Duplicated in row") {

    std::vector<int> sudoku{// @formatter:off
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,2,0,0,0,0,2,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0};
    // @formatter:on
    SatProblem *p = Encoder::Encode(3, sudoku);
    auto assignments = GetAssignments(sudoku, 3);
    REQUIRE(AnyFalse(p->clauses, assignments));
  }
  SECTION("Duplicated in subgrid") {

    std::vector<int> sudoku{// @formatter:off
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,4,0,0,0,0,0,
        0,0,0,0,4,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0};
    // @formatter:on
    SatProblem *p = Encoder::Encode(3, sudoku);
    auto assignments = GetAssignments(sudoku, 3);
    REQUIRE(AnyFalse(p->clauses, assignments));
  }
}

TEST_CASE("Test 3x3 Legal sudoku state", "[sudoku]") {
  SECTION("Partly solved") {

    std::vector<int> sudoku{// @formatter:off
        1,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,4,0,
        0,0,0,0,0,8,0,0,0,
        0,0,3,0,0,0,0,0,0,
        0,0,0,0,0,0,0,6,0,
        9,0,0,0,2,0,0,0,0,
        0,0,0,0,0,0,0,3,0,
        0,0,0,0,0,0,0,0,0,
        0,0,1,0,0,0,0,0,0};
    // @formatter:on
    SatProblem *p = Encoder::Encode(3, sudoku);
    auto assignments = GetAssignments(sudoku, 3);
    REQUIRE(!AnyFalse(p->clauses, assignments));
    REQUIRE(!AllTrue(p->clauses, assignments));
  }
  SECTION("Almost solved") {

    std::vector<int> sudoku{// @formatter:off
        1,7,4,9,6,3,8,5,2,
        2,8,5,1,7,4,9,6,3,
        3,9,6,2,8,5,1,7,4,
        4,1,7,3,9,6,2,8,5,
        5,2,8,4,1,7,3,9,6,
        6,3,9,5,2,8,4,1,7,
        7,4,1,6,3,0,5,2,8,
        8,5,2,7,4,1,6,3,9,
        9,6,3,8,5,2,7,4,1};
    // @formatter:on
    SatProblem *p = Encoder::Encode(3, sudoku);
    auto assignments = GetAssignments(sudoku, 3);
    REQUIRE(!AnyFalse(p->clauses, assignments));
    REQUIRE(!AllTrue(p->clauses, assignments));
  }
  SECTION("Completely solved") {
    std::vector<int> sudoku{// @formatter:off
        1,7,4,9,6,3,8,5,2,
        2,8,5,1,7,4,9,6,3,
        3,9,6,2,8,5,1,7,4,
        4,1,7,3,9,6,2,8,5,
        5,2,8,4,1,7,3,9,6,
        6,3,9,5,2,8,4,1,7,
        7,4,1,6,3,9,5,2,8,
        8,5,2,7,4,1,6,3,9,
        9,6,3,8,5,2,7,4,1};
    // @formatter:on
    SatProblem *p = Encoder::Encode(3, sudoku);
    auto assignments = GetAssignments(sudoku, 3);
    REQUIRE(!AnyFalse(p->clauses, assignments));
    REQUIRE(AllTrue(p->clauses, assignments));
 }
}

} // namespace simple_sat_solver::sudoku