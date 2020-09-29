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
/// Generate a sudoku by modeling the entire search path.
class GeneratorV2 {
public:
  /// Create a generator
  /// \param solver solver that can be used
  /// \param sub_size size of the subgrid.
  GeneratorV2(ISolver *solver, int sub_size)
      : solver_(solver), sub_size_(sub_size), size_(sub_size * sub_size){};
  ~GeneratorV2();
  /// Generate a sudoku with an unique solution that has revealed given cells.
  /// \param revealed the number of given cells
  /// \return a sudoku.
  sudoku::Sudoku Generate(int revealed);
  /// Convert the sat solution to a sudoku.
  /// \param sat_solution the sat solution that contains the entire solve path.
  /// \param state_index the index of the step that must be converted to a
  /// sudoku.
  /// \return the sudoku that was known at state state_index.
  sudoku::Sudoku SatToStartSudoku(std::vector<bool> sat_solution,
                                  int state_index);

  /// Get the sat index of the var
  inline int VarIndex(int offset, int x, int y, int value) const {
    return VarIndex(offset, CellIndex(x, y), value);
  }

  // Get the sat index of the var
  inline int VarIndex(int offset, int c, int value) const {
    return c * size_ + value + offset;
  }

  // Get the cell index.
  inline int CellIndex(int x, int y) const { return x + y * size_; }

private:
  /// Get the cell indices from row row_index.
  std::vector<int> GetRowCells(int row_index);
  /// Get the cell indices from column column_index.
  std::vector<int> GetColumnCells(int column_index);
  /// get the cell indices from subgrid subgrid_index.
  std::vector<int> GetSubgridCells(int subgrid_index);

  /// Create the start state.
  /// The start state has for revealed_cells cell one possible value.
  /// For all the other cells all the values are possible.
  /// \param problem sat problem to which the state must be added.
  /// \param revealed_cells number of cells with one value.
  void CreateStartState(sat::SatProblem &problem, int revealed_cells);
  /// Add a new state to problem and add the rules how the new state can be
  /// derived from the previous state.
  /// \param problem sat problem to which the state must be added.
  /// \param advanced_reasons true if it should use complicated reasons
  /// (slower).
  void CreateNextState(sat::SatProblem &problem, bool advanced_reasons);
  /// Add constraints that specify if a cell could not have a value in a
  /// previous state it can not have that value in the current state.
  void AddExcludedConstraints(sat::SatProblem &problem, int prev_start_index,
                              int start_index);
  /// Add constraint that specifies that a cell can eliminate a value if one of
  /// the reasons is true.
  /// \param problem problem to wich the constraint must be added.
  /// \param reasons reasons the can eliminate a value.
  void
  AddIncludedDomainConstraints(sat::SatProblem &problem,
                               std::vector<std::vector<sat::Lit>> &reasons);
  /// Init the default reason vector.
  /// \param prev_start_index sat index of the previous state.
  /// \param start_index sat index of the current state.
  /// \return
  std::vector<std::vector<sat::Lit>> IntitReasonVector(int prev_start_index,
                                                       int start_index);

  /// Add constraints to problem that specify that when a value is filled in, it
  /// can be remove from the domains it its row/column and subgrid.
  /// \param prev_start_index sat index of the previous state.
  /// \param reasons vector that keeps track of the reasons why a value can be
  /// removed from a cell.
  /// \param problem the problem to which to add the constraints.
  void AddUniqueConstraints(int prev_start_index,
                            std::vector<std::vector<sat::Lit>> &reasons,
                            sat::SatProblem &problem);

  /// Add constraints for a solved sudoku to the state starting at start_index.
  /// \param problem the problem to which to add the constraints.
  /// \param start_index the sat index of the state.
  void AddSolvedConstraints(sat::SatProblem &problem, int start_index);

  /// Specify that a solved cell should have a unique value.
  /// \param problem
  /// \param start_index
  void CreateSolvedCellConstraints(sat::SatProblem &problem, int start_index);
  /// Add the constraints that specifies the a value can be placed if there is
  /// only one cell in a row/column/subgrid that can receive that value.
  ///
  /// \param prev_start_index
  /// \param reasons
  /// \param problem
  void AddHiddenSingles(int prev_start_index,
                        std::vector<std::vector<sat::Lit>> &reasons,
                        sat::SatProblem &problem);

  /// If a value can only placed in one row of a subgrid, it can not appear
  /// anywhere else on the row or subgrid.
  /// \param prev_start_index
  /// \param
  /// reasons
  /// \param problem
  void AddRowGridConstraint(int prev_start_index,
                            std::vector<std::vector<sat::Lit>> &reasons,
                            sat::SatProblem &problem);

  /// If a value can only placed in one column of a subgrid, it can not appear
  /// anywhere else on the column or subgrid.
  void AddColumnGridConstraint(int prev_start_index,
                               std::vector<std::vector<sat::Lit>> &reasons,
                               sat::SatProblem &problem);

  /// A value can only appear once in the given cell_indices.
  void AddUniqueInGroupConstraint(sat::SatProblem &problem,
                                  std::vector<std::vector<sat::Lit>> &reasons,
                                  std::vector<int> cell_indices,
                                  int start_r_index);
  /// A value must only appear once in the given cell_indices.
  void CreateSolvedGroupConstraint(sat::SatProblem &problem,
                                   std::vector<int> cell_indices,
                                   int start_index);
  /// A value can be placed if there is only one cell in the group that can
  /// contain that value.
  void AddHiddenSingles(int prev_start_index, std::vector<int> cell_indices,
                        std::vector<std::vector<sat::Lit>> &reasons,
                        sat::SatProblem &problem);

  /// If for cause a value can only be placed on the intersection with effect,
  /// than effect can eliminate that value for the cells that are not part
  /// of the intersection.
  void AddGroupConstrainsGroup(int prev_start_index, std::vector<int> cause,
                               std::vector<int> effect,
                               std::vector<std::vector<sat::Lit>> &reasons,
                               sat::SatProblem &problem);
  /// if there are two cells that can contains only the values a and b.
  /// Then a and b must be placed in these cells and can be eliminated from the
  /// other cells.
  void AddPairGroup(int prev_start_index, std::vector<int> cell_indices,
                    std::vector<std::vector<sat::Lit>> &reasons,
                    sat::SatProblem &problem);
  /// Add the pair rules for the rows, columns and subgrids.
  void AddPairs(int prev_start_index,
                std::vector<std::vector<sat::Lit>> &reasons,
                sat::SatProblem &problem);

  const int sub_size_;
  const int size_;
  ISolver *solver_;
  std::vector<int> sudoku_start_indices_;
};
} // namespace simple_sat_solver::sudoku_generator
#endif // SIMPLESATSOLVER_SRC_SUDOKU_GENERATOR_GENERATOR_V_2_H_
