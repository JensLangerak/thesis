//
// Created by jens on 24-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_SUDOKU_GENERATOR_GENERATOR_V_2_H_
#define SIMPLESATSOLVER_SRC_SUDOKU_GENERATOR_GENERATOR_V_2_H_

#include "../sat/sat_problem.h"
#include "../sat/types.h"
#include "../solver_wrappers/i_solver.h"
#include "../sudoku/sudoku.h"
#include "sudoku_domain.h"

using simple_sat_solver::solver_wrappers::ISolver;
namespace simple_sat_solver::sudoku_generator {
class GeneratorV2 {
public:
  GeneratorV2(ISolver *solver, int sub_size)
      : solver_(solver), sub_size_(sub_size), size_(sub_size * sub_size){};
  ~GeneratorV2();
  sudoku::Sudoku Generate(int revealed);
  sudoku::Sudoku SatToStartSudoku(std::vector<bool> sat_solution,
                                  int board_index);

  inline int VarIndex(int offset, int x, int y, int value) const {
    return VarIndex(offset, CellIndex(x, y), value);
  }

  inline int VarIndex(int offset, int c, int value) const {
    return c * size_ + value + offset;
  }
  inline int CellIndex(int x, int y) const { return x + y * size_; }

private:
  const int sub_size_;
  const int size_;
  ISolver *solver_;
  std::vector<int> sudoku_start_indices_;

  std::vector<int> GetRowCells(int row_index);
  std::vector<int> GetColumnCells(int column_index);
  std::vector<int> GetSubgridCells(int subgrid_index);

  void CreateStartBoard(sat::SatProblem &problem, int revealed_cells);
  void CreateNextBoard(sat::SatProblem &problem, bool advanced_reasons);
  void AddExcludedConstraints(sat::SatProblem &problem, int prev_start_index,
                              int start_index);
  void
  AddIncludedDomainConstraints(sat::SatProblem &problem,
                               std::vector<std::vector<sat::Lit>> &reasons);
  std::vector<std::vector<sat::Lit>> IntitReasonVector(int prev_start_index,
                                                       int start_index);
  void AddUniqueConstraints(int prev_start_index, int start_index,
                            std::vector<std::vector<sat::Lit>> &reasons,
                            sat::SatProblem &problem);

  void AddSolvedConstraints(sat::SatProblem &problem, int start_index);

  void CreateCellConstraints(sat::SatProblem &problem, int start_index);
  void AddHiddenSingles(int prev_start_index, int start_index,
                        std::vector<std::vector<sat::Lit>> &reasons,
                        sat::SatProblem &problem);

  void AddRowGridConstraint(int prev_start_index, int start_index,
                            std::vector<std::vector<sat::Lit>> &reasons,
                            sat::SatProblem &problem);
  void AddColumnGridConstraint(int prev_start_index, int start_index,
                               std::vector<std::vector<sat::Lit>> &reasons,
                               sat::SatProblem &problem);

  void AddUniqueInGroupConstraint(sat::SatProblem &problem,
                                  std::vector<std::vector<sat::Lit>> &reasons,
                                  std::vector<int> cell_indices,
                                  int start_r_index);
  void CreateSolvedGroupConstraint(sat::SatProblem &problem,
                                   std::vector<int> cell_indices,
                                   int start_index);
  void AddHiddenSingles(int prev_start_index, std::vector<int> cell_indices,
                        std::vector<std::vector<sat::Lit>> &reasons,
                        sat::SatProblem &problem);
  void AddGroupConstrainsGroup(int prev_start_index, std::vector<int> cause,
                               std::vector<int> effect,
                               std::vector<std::vector<sat::Lit>> &reasons,
                               sat::SatProblem &problem);
  void AddPairGroup(int prev_start_index, std::vector<int> cell_indices,
                    std::vector<std::vector<sat::Lit>> &reasons,
                    sat::SatProblem &problem);
  void AddPairs(int prev_start_index,
                std::vector<std::vector<sat::Lit>> &reasons,
                sat::SatProblem &problem);
};
} // namespace simple_sat_solver::sudoku_generator
#endif // SIMPLESATSOLVER_SRC_SUDOKU_GENERATOR_GENERATOR_V_2_H_
