//
// Created by jens on 15-09-20.
//

#include "encoder.h"

namespace simple_sat_solver::sudoku {

SatProblem Encoder::Encode(const Sudoku &sudoku) {
  Encoder e(sudoku.sub_size);
  e.CreateCellConstraints();
  e.CreateRowConstraints();
  e.CreateColumnConstraints();
  e.CreateSubGridConstraints();
  e.AddGivenConstraints(sudoku.cells);
  return e.problem_;
}
void Encoder::CreateCellConstraints() {
  for (int x = 0; x < size_; x++) {
    for (int y = 0; y < size_; y++) {
      std::vector<int> exactly_one;
      exactly_one.reserve(size_);
      for (int v = 1; v <= size_; v++) {
        exactly_one.push_back(VarIndex(x, y, v));
      }
      CreateUniqueConstraints(exactly_one);
    }
  }
}
void Encoder::CreateRowConstraints() {
  for (int x = 0; x < size_; x++) {
    for (int v = 1; v <= size_; v++) {
      std::vector<int> exactly_one;
      exactly_one.reserve(size_);
      for (int y = 0; y < size_; y++) {
        exactly_one.push_back(VarIndex(x, y, v));
      }
      CreateUniqueConstraints(exactly_one);
    }
  }
}
void Encoder::CreateColumnConstraints() {
  for (int y = 0; y < size_; y++) {
    for (int v = 1; v <= size_; v++) {
      std::vector<int> exactly_one;
      exactly_one.reserve(size_);
      for (int x = 0; x < size_; x++) {
        exactly_one.push_back(VarIndex(x, y, v));
      }
      CreateUniqueConstraints(exactly_one);
    }
  }
}
void Encoder::CreateSubGridConstraints() {
  for (int v = 1; v <= size_; v++) {
    for (int sub_x = 0; sub_x < sub_size_; sub_x++) {
      for (int sub_y = 0; sub_y < sub_size_; sub_y++) {
        std::vector<int> exactly_one;
        exactly_one.reserve(size_);
        for (int x = 0; x < sub_size_; x++) {
          for (int y = 0; y < sub_size_; y++) {
            exactly_one.push_back(
                VarIndex(x + sub_x * sub_size_, y + sub_y * sub_size_, v));
          }
        }
        CreateUniqueConstraints(exactly_one);
      }
    }
  }
}
void Encoder::CreateUniqueConstraints(const std::vector<int> &vars) {
  std::vector<solver::Lit> at_least_one;
  at_least_one.reserve(vars.size());
  for (int var : vars) {
    at_least_one.emplace_back(var, false);
  }
  problem_.clauses.push_back(at_least_one);
  for (int i = 0; i < vars.size() - 1; i++) {
    for (int j = i + 1; j < vars.size(); j++) {
      std::vector<solver::Lit> at_most_one;
      at_most_one.emplace_back(vars[i], true);
      at_most_one.emplace_back(vars[j], true);
      problem_.clauses.push_back(at_most_one);
    }
  }
}
void Encoder::AddGivenConstraints(const std::vector<int> &vars) {
  for (int i = 0; i < vars.size(); i++) {
    if (vars[i] < 1)
      continue;
    int x = i % size_;
    int y = i / size_;
    std::vector<solver::Lit> unit;
    unit.emplace_back(VarIndex(x, y, vars[i]), false);
    problem_.clauses.push_back(unit);
  }
}
Sudoku Encoder::Decode(int sub_size, std::vector<solver::LBool> &solution) {
  int size = sub_size * sub_size;
  std::vector<int> res;
  res.reserve(size * size);
  for (int i = 0; i < size * size; i++) {
    bool added = false;
    for (int v = 1; v <= size; v++) {
      if (solution[i * size + v - 1] == solver::LBool::kTrue) {
        if (added)
          throw "Solution is not valid!";
        added = true;
        res.push_back(v);
      }
    }
    if (!added)
      res.push_back(-1);
  }

  return Sudoku(sub_size, res);
}
Sudoku Encoder::Decode(int sub_size, std::vector<bool> &solution) {
  int size = sub_size * sub_size;
  std::vector<int> res;
  res.reserve(size * size);
  for (int i = 0; i < size * size; i++) {
    bool added = false;
    for (int v = 1; v <= size; v++) {
      if (solution[i * size + v - 1]) {
        if (added)
          throw "Solution is not valid!";
        added = true;
        res.push_back(v);
      }
    }
    if (!added)
      res.push_back(-1);
  }

  return Sudoku(sub_size, res);
}
} // namespace simple_sat_solver::sudoku