//
// Created by jens on 15-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_ENCODER_H_
#define SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_ENCODER_H_

#include "../solver/types.h"
#include "types.h"

namespace simple_sat_solver::sudoku {
class Encoder {
public:
  static SatProblem Encode(const Sudoku &sudoku);

  // TODO methods are currently duplicated
  static Sudoku Decode(int sub_size, std::vector<solver::LBool> &solution);
  static Sudoku Decode(int sub_size, std::vector<bool> &solution);

private:
  explicit inline Encoder(int sub_size)
      : sub_size_(sub_size), size_(sub_size * sub_size),
        problem_(SatProblem(size_ * size_ * size_)){};

  void CreateCellConstraints();
  void CreateRowConstraints();
  void CreateColumnConstraints();
  void CreateSubGridConstraints();
  void CreateUniqueConstraints(const std::vector<int> &vars);
  void AddGivenConstraints(const std::vector<int> &vars);

  inline int VarIndex(int x, int y, int value) const {
    return (x + y * size_) * size_ + value - 1;
  }

  int sub_size_;
  int size_;
  SatProblem problem_;
};
} // namespace simple_sat_solver::sudoku

#endif // SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_ENCODER_H_
