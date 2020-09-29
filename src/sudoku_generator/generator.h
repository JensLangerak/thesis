//
// Created by jens on 23-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_SUDOKU_GENERATOR_GENERATOR_H_
#define SIMPLESATSOLVER_SRC_SUDOKU_GENERATOR_GENERATOR_H_

#include "../solver_wrappers/i_solver.h"
#include "../sudoku/encoder.h"
#include "../sudoku/sudoku.h"

using simple_sat_solver::sudoku::Sudoku;
namespace simple_sat_solver::sudoku_generator {
/// Sudoku generator that starts with an empty sudoku and keeps adding numbers
/// until it has a unique solution. After that is tries to alter the givens to
/// match the desired amount. It uses sat to test for uniqueness, adding or
/// removing of given cells is done outside sat.
class Generator {
public:
  /// Create a new sudoku generator
  /// \param solver solver that can be used.
  explicit Generator(solver_wrappers::ISolver *solver);
  ~Generator();
  /// Generate a sudoku.
  /// \param sub_size size of the subgrids in the sudoku
  /// \param given number of given celss.
  /// \return A sudoku of the desired size, with the given filled in cells.
  Sudoku Generate(int sub_size, int given);

private:
  /// Get the indices of the elements that differ.
  /// \param sol1 Solved sudoku
  /// \param sol2 Another solved sudoku
  /// \return an array that contains the cell indices that are different.
  std::vector<int> DifferenceIndex(const Sudoku &sol1,
                                   const Sudoku &sol2) const;
  /// Generate a sudoku with a unique solution of the given size.
  /// \param sub_size size of the subgrid.
  /// \return a sudoku with a unique solution.
  Sudoku GenerateUnique(int sub_size);
  /// Generate a sat problem that is satisfiable if there exist a solution to
  /// problem that is not equal to vector.
  ///
  /// \param problem the problem for which we want to find a different solution.
  /// \param vector a solution that was already found for the problem.
  /// \return A sat problem that is satisfiable if there exist another solution.
  sat::SatProblem CreateSecondSolutionProblem(const sat::SatProblem &problem,
                                              const std::vector<bool> &vector);
  /// Reveal reveal_cells extra cells in problem.
  /// \param problem sudoku where the extra cells must be revealed.
  /// \param solution the solution to the sudoku.
  /// \param reveal_cells the number of extra cells that should be revealed.
  /// \return true if succeeded.
  bool RevealExtraCells(Sudoku &problem, Sudoku solution, int reveal_cells);
  /// Try to hide remove_givens cells from the problem, while keeping the
  /// solution unique.
  ///
  /// \param problem the sudoku where the cells must be hidden.
  /// \param solution the solution to the sudoku.
  /// \param remove_givens the number of cells that must be removed.
  /// \return true if succeeded.
  bool ReduceSolution(Sudoku &problem, Sudoku solution, int remove_givens);
  /// Rest if solution is the only solution to problem.
  /// \param problem the sudoku.
  /// \param solution the solution to problem.
  /// \return true if solution is the only solution for the problem.
  bool SolutionIsUnique(Sudoku problem, Sudoku solution);
  /// Count how many cells are filled in.
  /// \param sudoku
  /// \return the number of filled in cells in sudoku.
  int GetFilledCells(Sudoku sudoku);
  /// Brute force search for a start sudoku with the given solution with given
  /// revealed cells.
  ///
  /// \param problem the sudoku.
  /// \param sudoku solution to problem.
  /// \param given
  /// \return true if succeeded.
  bool ExhaustiveSearch(Sudoku &problem, Sudoku sudoku, int given);

  /// Recursive method for the exhaustive search.
  /// \param problem the current constructed sudoku.
  /// \param solution the solution to the sudoku.
  /// \param sat_solution the sat solution of the sudoku.
  /// \param next_cell the lowest allowed cell index for the next revealed cell.
  /// \param unassigned_givens the number of cells that must be revealed.
  /// \return true if it can construct a problem with the desired givens and an
  /// unique solution.
  bool MinSolution(Sudoku &problem, Sudoku solution,
                   std::vector<bool> sat_solution, int next_cell,
                   int unassigned_givens);

  /// Test if solution is the only solution for problem.
  /// \param problem
  /// \param solution
  /// \return true if solution is the only solution to problem.
  bool SolutionIsUnique(sat::SatProblem problem, std::vector<bool> solution);
  /// First remove max_nr of cells from the problem. Next request a number of
  /// solutions to problem and add all the cells that can have multiple values.
  /// The value of the given cells is retrieved from solution.
  /// \param problem the sudoku
  /// \param solution the solution to sudoku
  /// \param sat_solution the sat solution to the sat problem from sudoku.
  /// \param max_nr
  /// \return true if succeeded.
  bool ExpandSolution(Sudoku &problem, Sudoku solution,
                      std::vector<bool> sat_solution, int max_nr);

  solver_wrappers::ISolver *solver_;
  sudoku::Encoder encoder_;
};
} // namespace simple_sat_solver::sudoku_generator

#endif // SIMPLESATSOLVER_SRC_SUDOKU_GENERATOR_GENERATOR_H_
