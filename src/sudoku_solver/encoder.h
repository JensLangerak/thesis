//
// Created by jens on 15-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_ENCODER_H_
#define SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_ENCODER_H_

#include "../solver/types.h"
#include "types.h"

namespace simple_sat_solver::sudoku {
// TODO decide how to split encoding/decoding, maybe call it convert and remove
// statics?
class Encoder {
  /// The encoding of the sudoku works as follows.
  /// Each cell has one var per possible value. Thus a var is indexed using 3
  /// parameters x,y and v. x and y correspond to the coord of the cell and v to
  /// the value.
  /// For example suppose that we have a 9x9 (subgrid size 3) sudoku.
  /// For each cell we have 9 vars. Suppose cell (2,4) has value 8.
  /// Then var_2_4_8 = True and var_2_4_i for i 1..7,9 is False.
public:
  /// Encode the sudoku as a SAT problem.
  /// \param sudoku the sudoku that must be encoded.
  /// \return a SAT problem that is satisfiable iff the sudoku is solvable.
  static SatProblem Encode(const Sudoku &sudoku);

  // TODO methods are currently duplicated
  /// Convert the SAT solution back to the sudoku.
  /// \param sub_size size of the subgrid in the sudoku (for a 9x9 sudoku this
  /// is 3) \param solution the solution for a sudoku found by a SAT solver.
  /// \return a filled in sudoku based on the found solution.
  static Sudoku Decode(int sub_size, std::vector<solver::LBool> &solution);
  static Sudoku Decode(int sub_size, std::vector<bool> &solution);

private:
  /// Create a new encoder.
  /// \param sub_size size of the sub grid in the sudoku.
  explicit inline Encoder(int sub_size)
      : sub_size_(sub_size), size_(sub_size * sub_size),
        problem_(SatProblem(size_ * size_ * size_)){};

  /// Add the constraints that each cell must have 1 filled in number.
  void CreateCellConstraints();
  /// Add the constraints that each number in a row must be unique.
  void CreateRowConstraints();
  /// Add the constraints that each number in a column must be unique.
  void CreateColumnConstraints();
  /// Add the constraints that each number is a subgrid must be unique.
  void CreateSubGridConstraints();

  /// Add constraints that specifies that exactly one var of vars must be true.
  /// \param vars a group of vars where exactly one var must be true.
  void CreateUniqueConstraints(const std::vector<int> &vars);
  /// Add the constraints that force the given numbers.
  /// \param vars the cells of a unsolved sudoku. Unrevealed cells have a value
  /// < 1.
  void AddGivenConstraints(const std::vector<int> &vars);

  /// Compute the index of the var that corresponds to the given parameters.
  /// \param x x-coord of the cell.
  /// \param y y-coord of the cell.
  /// \param value potential value of the cell.
  /// \return the index of the var that correspoinds to the given paramets.
  inline int VarIndex(int x, int y, int value) const {
    return (x + y * size_) * size_ + value - 1;
  }

  int sub_size_;
  int size_;
  SatProblem problem_;
};
} // namespace simple_sat_solver::sudoku

#endif // SIMPLESATSOLVER_SRC_SUDOKU_SOLVER_ENCODER_H_
