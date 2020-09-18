//
// Created by jens on 16-09-20.
//
#include <algorithm>

#include "lib/catch2/catch.hpp"
#include "src/sudoku/encoder.h"

namespace simple_sat_solver::sudoku {
bool IsFalse(const std::vector<solver::Lit> &clause,
             const std::vector<solver::LBool> &values) {
  for (auto l : clause) {
    solver::LBool value = values[l.x];
    if (value == solver::LBool::kUnknown ||
        value == solver::LBool::kTrue && (!l.complement) ||
        value == solver::LBool::kFalse && l.complement)
      return false;
  }
  return true;
}

bool IsTrue(const std::vector<solver::Lit> &clause,
            const std::vector<solver::LBool> &values) {
  for (auto l : clause) {
    solver::LBool value = values[l.x];
    if (value == solver::LBool::kTrue && (!l.complement) ||
        value == solver::LBool::kFalse && l.complement)
      return true;
  }
  return false;
}

bool AnyTrue(const std::vector<std::vector<solver::Lit>> &clauses,
             const std::vector<solver::LBool> &values) {
  for (auto c : clauses) {
    if (IsTrue(c, values))
      return true;
  }
  return false;
}
bool AnyFalse(const std::vector<std::vector<solver::Lit>> &clauses,
              const std::vector<solver::LBool> &values) {
  for (auto c : clauses) {
    if (IsFalse(c, values))
      return true;
  }
  return false;
}
bool AllTrue(const std::vector<std::vector<solver::Lit>> &clauses,
             const std::vector<solver::LBool> &values) {
  for (auto c : clauses) {
    if (!IsTrue(c, values))
      return false;
  }
  return true;
}

std::vector<solver::LBool> GetAssignments(Sudoku &s) {
  int size = s.sub_size * s.sub_size;
  std::vector<solver::LBool> assignments;
  assignments.reserve(size * size * size);
  for (int c : s.cells) {
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
      Sudoku sudoku(2, {// @formatter:off
          0,0,0,0,
          0,0,0,0,
          0,0,0,0,
          0,0,0,0});
      // @formatter:on
      REQUIRE(GetAssignments(sudoku).size() == 4 * 4 * 4);
    }
    SECTION("3 (9x9)") {
      Sudoku sudoku(3, {// @formatter:off
          0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0});
      // @formatter:on
      REQUIRE(GetAssignments(sudoku).size() == 9 * 9 * 9);
    }
  }
  SECTION("Assigned values") {
    Sudoku sudoku(2, {// @formatter:off
        -1,0,0,0,
        0,2,0,0,
        0,0,0,0,
        0,0,4,1});
    // @formatter:on
    auto assignments = GetAssignments(sudoku);
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
  Sudoku sudoku(2,{// @formatter:off
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0});
  // @formatter:on
  sat::SatProblem *p = Encoder::Encode(sudoku);
  auto assignments = GetAssignments(sudoku);
  SECTION("Size") { REQUIRE(p->GetNrVars() == 4 * 4 * 4); }

  SECTION("All lits uses existing vars") {
    for (const auto &c : p->GetClauses()) {
      for (auto l : c) {
        REQUIRE(l.x >= 0);
        REQUIRE(l.x < 4 * 4 * 4);
      }
    }
  }

  SECTION("Currently solvable") {
    REQUIRE(AnyFalse(p->GetClauses(), assignments) == false);
  }

  delete p;
}
TEST_CASE("Test 2x2 illegal states", "[sudoku]") {
  SECTION("Duplicated filled in cell") {
    Sudoku sudoku(2,{// @formatter:off
        2, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0});
    // @formatter:on
    sat::SatProblem *p = Encoder::Encode(sudoku);
    auto assignments = GetAssignments(sudoku);
    assignments[0] = solver::LBool::kTrue;
    REQUIRE(AnyFalse(p->GetClauses(), assignments));
    delete p;
  }
  SECTION("Duplicated in column") {
    Sudoku sudoku(2,{// @formatter:off
        2, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        2, 0, 0, 0});
    // @formatter:on
    sat::SatProblem *p = Encoder::Encode(sudoku);
    auto assignments = GetAssignments(sudoku);
    REQUIRE(AnyFalse(p->GetClauses(), assignments));
    delete p;
  }
  SECTION("Duplicated in row") {
    Sudoku sudoku(2,{// @formatter:off
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 3, 0, 3,
        0, 0, 0, 0});
    // @formatter:on
    sat::SatProblem *p = Encoder::Encode(sudoku);
    auto assignments = GetAssignments(sudoku);
    REQUIRE(AnyFalse(p->GetClauses(), assignments));
    delete p;
  }
  SECTION("Duplicated in subgrid") {
    Sudoku sudoku(2, {// @formatter:off
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 4, 0,
        0, 0, 0, 4});
    // @formatter:on
    sat::SatProblem *p = Encoder::Encode(sudoku);
    auto assignments = GetAssignments(sudoku);
    REQUIRE(AnyFalse(p->GetClauses(), assignments));
    delete p;
  }
}

TEST_CASE("Test 2x2 Legal sudoku state", "[sudoku]") {
  SECTION("Partly solved") {
    Sudoku sudoku(2, {// @formatter:off
        2, 0, 0, 0,
        4, 0, 2, 0,
        0, 3, 0, 1,
        1, 0, 4, 0});
    // @formatter:on
    sat::SatProblem *p = Encoder::Encode(sudoku);
    auto assignments = GetAssignments(sudoku);
    REQUIRE(!AnyFalse(p->GetClauses(), assignments));
    REQUIRE(!AllTrue(p->GetClauses(), assignments));
    delete p;
  }
  SECTION("Almost solved") {
    Sudoku sudoku(2, {// @formatter:off
        1, 3, 4, 2,
        2, 4, 1, 3,
        3, 1, -1, 4,
        4, 2, 3, 1});
    // @formatter:on
    sat::SatProblem *p = Encoder::Encode(sudoku);
    auto assignments = GetAssignments(sudoku);
    REQUIRE(!AnyFalse(p->GetClauses(), assignments));
    REQUIRE(!AllTrue(p->GetClauses(), assignments));
    delete p;
  }
  SECTION("Completely solved") {
    Sudoku sudoku(2, {// @formatter:off
        1, 3, 4, 2,
        2, 4, 1, 3,
        3, 1, 2, 4,
        4, 2, 3, 1});
    // @formatter:on
    sat::SatProblem *p = Encoder::Encode(sudoku);
    auto assignments = GetAssignments(sudoku);
    REQUIRE(AnyFalse(p->GetClauses(), assignments) == false);
    REQUIRE(AllTrue(p->GetClauses(), assignments));
    delete p;
  }
}

TEST_CASE("Test 3x3 structure", "[sudoku]") {
  Sudoku sudoku(3, {// @formatter:off
      0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0});
  // @formatter:on
  sat::SatProblem *p = Encoder::Encode(sudoku);
  auto assignments = GetAssignments(sudoku);
  SECTION("Size") { REQUIRE(p->GetNrVars() == 9 * 9 * 9); }

  SECTION("All lits uses existing vars") {
    for (const auto &c : p->GetClauses()) {
      for (auto l : c) {
        REQUIRE(l.x >= 0);
        REQUIRE(l.x < 9 * 9 * 9);
      }
    }
  }

  SECTION("Currently solvable") {
    REQUIRE(AnyFalse(p->GetClauses(), assignments) == false);
  }

  delete p;
}
TEST_CASE("Test 3x3 illegal states", "[sudoku]") {
  SECTION("Duplicated filled in cell") {

    Sudoku sudoku(3, {// @formatter:off
        2,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0});
    // @formatter:on
    sat::SatProblem *p = Encoder::Encode(sudoku);
    auto assignments = GetAssignments(sudoku);
    assignments[0] = solver::LBool::kTrue;
    REQUIRE(AnyFalse(p->GetClauses(), assignments));
    delete p;
  }
  SECTION("Duplicated in column") {

    Sudoku sudoku(3, {// @formatter:off
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,1,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,1,0,0,0,0,0});
    // @formatter:on
    sat::SatProblem *p = Encoder::Encode(sudoku);
    auto assignments = GetAssignments(sudoku);
    REQUIRE(AnyFalse(p->GetClauses(), assignments));
    delete p;
  }
  SECTION("Duplicated in row") {

    Sudoku sudoku(3, {// @formatter:off
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,2,0,0,0,0,2,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0});
    // @formatter:on
    sat::SatProblem *p = Encoder::Encode(sudoku);
    auto assignments = GetAssignments(sudoku);
    REQUIRE(AnyFalse(p->GetClauses(), assignments));
    delete p;
  }
  SECTION("Duplicated in subgrid") {

    Sudoku sudoku(3, {// @formatter:off
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,4,0,0,0,0,0,
        0,0,0,0,4,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0});
    // @formatter:on
    sat::SatProblem *p = Encoder::Encode(sudoku);
    auto assignments = GetAssignments(sudoku);
    REQUIRE(AnyFalse(p->GetClauses(), assignments));

    delete p;
  }
}

TEST_CASE("Test 3x3 Legal sudoku state", "[sudoku]") {
  SECTION("Partly solved") {

    Sudoku sudoku(3, {// @formatter:off
        1,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,4,0,
        0,0,0,0,0,8,0,0,0,
        0,0,3,0,0,0,0,0,0,
        0,0,0,0,0,0,0,6,0,
        9,0,0,0,2,0,0,0,0,
        0,0,0,0,0,0,0,3,0,
        0,0,0,0,0,0,0,0,0,
        0,0,1,0,0,0,0,0,0});
    // @formatter:on
    sat::SatProblem *p = Encoder::Encode(sudoku);
    auto assignments = GetAssignments(sudoku);
    REQUIRE(!AnyFalse(p->GetClauses(), assignments));
    REQUIRE(!AllTrue(p->GetClauses(), assignments));

    delete p;
  }
  SECTION("Almost solved") {

    Sudoku sudoku(3, {// @formatter:off
        1,7,4,9,6,3,8,5,2,
        2,8,5,1,7,4,9,6,3,
        3,9,6,2,8,5,1,7,4,
        4,1,7,3,9,6,2,8,5,
        5,2,8,4,1,7,3,9,6,
        6,3,9,5,2,8,4,1,7,
        7,4,1,6,3,0,5,2,8,
        8,5,2,7,4,1,6,3,9,
        9,6,3,8,5,2,7,4,1});
    // @formatter:on
    sat::SatProblem *p = Encoder::Encode(sudoku);
    auto assignments = GetAssignments(sudoku);
    REQUIRE(!AnyFalse(p->GetClauses(), assignments));
    REQUIRE(!AllTrue(p->GetClauses(), assignments));

    delete p;
  }
  SECTION("Completely solved") {
    Sudoku sudoku(3, {// @formatter:off
        1,7,4,9,6,3,8,5,2,
        2,8,5,1,7,4,9,6,3,
        3,9,6,2,8,5,1,7,4,
        4,1,7,3,9,6,2,8,5,
        5,2,8,4,1,7,3,9,6,
        6,3,9,5,2,8,4,1,7,
        7,4,1,6,3,9,5,2,8,
        8,5,2,7,4,1,6,3,9,
        9,6,3,8,5,2,7,4,1});
    // @formatter:on
    sat::SatProblem *p = Encoder::Encode(sudoku);
    auto assignments = GetAssignments(sudoku);
    REQUIRE(!AnyFalse(p->GetClauses(), assignments));
    REQUIRE(AllTrue(p->GetClauses(), assignments));
    delete p;
  }
}
TEST_CASE("Test decoder", "[sudoku]") {
  SECTION("Completely solved") {
    Sudoku sudoku(3, {// @formatter:off
        1,7,4,9,6,3,8,5,2,
        2,8,5,1,7,4,9,6,3,
        3,9,6,2,8,5,1,7,4,
        4,1,7,3,9,6,2,8,5,
        5,2,8,4,1,7,3,9,6,
        6,3,9,5,2,8,4,1,7,
        7,4,1,6,3,9,5,2,8,
        8,5,2,7,4,1,6,3,9,
        9,6,3,8,5,2,7,4,1});
    // @formatter:on
    auto assignments = GetAssignments(sudoku);
    Sudoku decoded = Encoder::Decode(3, assignments);
    REQUIRE(sudoku == decoded);
  }

  SECTION("Partial solved") {
    Sudoku sudoku(2, {// @formatter:off
        1, 3, 4, 2,
        2, 4, 1, 3,
        3, 1, -1, 4,
        4, 2, 3, 1});
    // @formatter:on
    auto assignments = GetAssignments(sudoku);
    Sudoku decoded = Encoder::Decode(2, assignments);
    REQUIRE(sudoku.sub_size == decoded.sub_size);
    for (int i = 0; i < sudoku.cells.size(); i++) {
      if (sudoku.cells[i] < 1)
        REQUIRE(decoded.cells[i] < 1);
      else
        REQUIRE(sudoku.cells[i] == decoded.cells[i]);
    }
  }
  SECTION("Illegal assignment") {
    Sudoku sudoku(2, {// @formatter:off
      1, 3, 4, 2,
      2, 4, 1, 3, 
      3, 1, 2, 4, 
      4, 2, 3, 1});
    // @formatter:on
    auto assignments = GetAssignments(sudoku);
    REQUIRE(assignments[2] == solver::LBool::kFalse);
    assignments[2] = solver::LBool::kTrue;
    REQUIRE_THROWS(Encoder::Decode(2, assignments));
  }
}
} // namespace simple_sat_solver::sudoku