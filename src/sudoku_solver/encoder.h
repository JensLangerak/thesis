//
// Created by jens on 15-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_ENCODER_H_
#define SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_ENCODER_H_

#include "../solver/types.h"
#include "sat_problem.h"

namespace simple_sat_solver::sudoku {
class Encoder {
public:
  static SatProblem *Encode(int subSize, std::vector<int> numbers);

private:
  explicit inline Encoder(int subSize)
      : subSize_(subSize), size_(subSize * subSize),
        problem_(new SatProblem(size_)){};

  void CreateCellConstraints();
  void CreateRowConstraints();
  void CreateColumnConstraints();
  void CreateSubGridConstraints();
  void CreateUniqueConstraints(const std::vector<int> &vars);
  void AddGivenConstraints(const std::vector<int> vars);

  inline int VarIndex(int x, int y, int value) const {
    return (x + y * size_) * size_ + value - 1;
  }

  int subSize_;
  int size_;
  SatProblem *problem_;
};
} // namespace simple_sat_solver::sudoku

#endif // SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_ENCODER_H_
