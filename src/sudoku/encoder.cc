//
// Created by jens on 15-09-20.
//

#include "encoder.h"

namespace simple_sat_solver::sudoku {

sat::SatProblem Encoder::Encode(const Sudoku &sudoku) {
  if (sudoku.sub_size != sub_size_)
    throw "Encoder is for different sized sudoku!";
  problem_= sat::SatProblem(size_ * size_ * size_);
  CreateCellConstraints();
  CreateRowConstraints();
  CreateColumnConstraints();
  CreateSubGridConstraints();
  AddGivenConstraints(sudoku.cells);
  return problem_;
}
void Encoder::CreateCellConstraints() {
  for (int x = 0; x < size_; x++) {
    for (int y = 0; y < size_; y++) {
      std::vector<sat::Lit> exactly_one;
      exactly_one.reserve(size_);
      for (int v = 1; v <= size_; v++) {
        exactly_one.emplace_back(VarIndex(x, y, v), false);
      }
      problem_.ExactlyOne(exactly_one);
    }
  }
}
void Encoder::CreateRowConstraints() {
  for (int x = 0; x < size_; x++) {
    for (int v = 1; v <= size_; v++) {
      std::vector<sat::Lit> exactly_one;
      exactly_one.reserve(size_);
      for (int y = 0; y < size_; y++) {
        exactly_one.emplace_back(VarIndex(x, y, v), false);
      }
      problem_.ExactlyOne(exactly_one);
    }
  }
}
void Encoder::CreateColumnConstraints() {
  for (int y = 0; y < size_; y++) {
    for (int v = 1; v <= size_; v++) {
      std::vector<sat::Lit> exactly_one;
      exactly_one.reserve(size_);
      for (int x = 0; x < size_; x++) {
        exactly_one.emplace_back(VarIndex(x, y, v), false);
      }
      problem_.ExactlyOne(exactly_one);
    }
  }
}
void Encoder::CreateSubGridConstraints() {
  for (int v = 1; v <= size_; v++) {
    for (int sub_x = 0; sub_x < sub_size_; sub_x++) {
      for (int sub_y = 0; sub_y < sub_size_; sub_y++) {
        std::vector<sat::Lit> exactly_one;
        exactly_one.reserve(size_);
        for (int x = 0; x < sub_size_; x++) {
          for (int y = 0; y < sub_size_; y++) {
            exactly_one.emplace_back(
                VarIndex(x + sub_x * sub_size_, y + sub_y * sub_size_, v), false);
          }
        }
        problem_.ExactlyOne(exactly_one);
      }
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
    problem_.AddClause(unit);
  }
}
Sudoku Encoder::Decode(std::vector<solver::LBool> &solution) {
  std::vector<int> res;
  res.reserve(size_ * size_);
  for (int i = 0; i < size_ * size_; i++) {
    bool added = false;
    for (int v = 1; v <= size_; v++) {
      if (solution[i * size_ + v - 1] == solver::LBool::kTrue) {
        if (added)
          throw "Solution is not valid!";
        added = true;
        res.push_back(v);
      }
    }
    if (!added)
      res.push_back(-1);
  }

  return Sudoku(sub_size_, res);
}
Sudoku Encoder::Decode(std::vector<bool> &solution) {
  std::vector<int> res;
  res.reserve(size_ * size_);
  for (int i = 0; i < size_ * size_; i++) {
    bool added = false;
    for (int v = 1; v <= size_; v++) {
      if (solution[i * size_ + v - 1]) {
        if (added)
          throw "Solution is not valid!";
        added = true;
        res.push_back(v);
      }
    }
    if (!added)
      res.push_back(-1);
  }

  return Sudoku(sub_size_, res);
}
} // namespace simple_sat_solver::sudoku