//
// Created by jens on 15-09-20.
//

#include "encoder.h"

namespace simple_sat_solver::sudoku {

solver::Solver *Encoder::Encode(int subSize, std::vector<int> numbers) {
  Encoder e(subSize);
  e.CreateVars();
  e.CreateCellConstraints();
  e.CreateRowConstraints();
  e.CreateColumnConstraints();
  e.CreateSubGridConstraints();
  e.AddGivenConstraints(numbers);
  return e.solver_;
}
void Encoder::CreateVars() {
  // get largest index
  for (int i = 0; i <= VarIndex(size_ - 1, size_ - 1, size_); i++) {
    solver_->NewVar();
  }
}
void Encoder::CreateCellConstraints() {
  for (int x = 0; x < size_; x++) {
    for (int y = 0; y < size_; y++) {
      std::vector<int> exactlyOne;
      exactlyOne.reserve(size_);
      for (int v = 1; v <= size_; v++) {
        exactlyOne.push_back(VarIndex(x, y, v));
      }
      CreateUniqueConstraints(exactlyOne);
    }
  }
}
void Encoder::CreateRowConstraints() {
  for (int x = 0; x < size_; x++) {
    for (int v = 1; v <= size_; v++) {
      std::vector<int> exactlyOne;
      exactlyOne.reserve(size_);
      for (int y = 0; y < size_; y++) {
        exactlyOne.push_back(VarIndex(x, y, v));
      }
      CreateUniqueConstraints(exactlyOne);
    }
  }
}
void Encoder::CreateColumnConstraints() {
  for (int y = 0; y < size_; y++) {
    for (int v = 1; v <= size_; v++) {
      std::vector<int> exactlyOne;
      exactlyOne.reserve(size_);
      for (int x = 0; x < size_; x++) {
        exactlyOne.push_back(VarIndex(x, y, v));
      }
      CreateUniqueConstraints(exactlyOne);
    }
  }
}
void Encoder::CreateSubGridConstraints() {
  for (int v = 1; v <= size_; v++) {
    for (int subX = 0; subX < subSize_; subX++) {
      for (int subY = 0; subY < subSize_; subY++) {
        std::vector<int> exactlyOne;
        exactlyOne.reserve(size_);
        for (int x = 0; x < subSize_; x++) {
          for (int y = 0; y < subSize_; y++) {
            exactlyOne.push_back(
                VarIndex(x + subX * subSize_, y + subY * subSize_, v));
          }
        }
        CreateUniqueConstraints(exactlyOne);
      }
    }
  }
}
void Encoder::CreateUniqueConstraints(const std::vector<int> &vars) {
  std::vector<solver::Lit> atLeastOne;
  for (int var : vars) {
    atLeastOne.emplace_back(var, false);
  }
  solver_->AddClause(atLeastOne);
  for (int i = 0; i < vars.size() - 1; i++) {
    for (int j = i + 1; j < vars.size(); j++) {
      std::vector<solver::Lit> atMostOne;
      atMostOne.emplace_back(vars[i], true);
      atMostOne.emplace_back(vars[j], true);
      solver_->AddClause(atMostOne);
    }
  }
}
void Encoder::AddGivenConstraints(const std::vector<int> vars) {
  for (int i = 0; i < vars.size(); i++) {
    if (vars[i] < 1)
      continue;
    int x = i % size_;
    int y = i / size_;
    std::vector<solver::Lit> unit;
    unit.emplace_back(VarIndex(x, y, vars[i]), false);
    solver_->AddClause(unit);
  }
}
} // namespace simple_sat_solver::sudoku