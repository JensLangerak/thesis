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
class Generator {
public:
  explicit Generator(solver_wrappers::ISolver *solver);
  ~Generator();
  Sudoku Generate(int sub_size, int given);

private:
  solver_wrappers::ISolver *solver_;
  sudoku::Encoder encoder;

  std::vector<int> DifferenceIndex(const Sudoku &sol1,
                                   const Sudoku &sol2) const;
  Sudoku GenerateUnique(int sub_size);
  sat::SatProblem CreateSecondSolutionProblem(const sat::SatProblem &problem,
                                              const std::vector<bool> &vector);
  bool RevealExtraCells(Sudoku &problem, Sudoku solution, int reveal_cells);
  bool ReduceSolution(Sudoku &problem, Sudoku solution, int remove_givens);
  bool SolutionIsUnique(Sudoku problem, Sudoku solution);
  int GetFilledCells(Sudoku sudoku);
  bool ExhaustiveSearch(Sudoku &problem, Sudoku sudoku, int given);
  bool MinSolution(Sudoku &problem, Sudoku solution,
                   std::vector<bool> sat_solution, int next_cell,
                   int unassigned_givens);
  bool SolutionIsUnique(sat::SatProblem problem, std::vector<bool> solution);
  bool ExpandSolution(Sudoku &problem, Sudoku solution,
                      std::vector<bool> sat_solution, int max_nr);
};
} // namespace simple_sat_solver::sudoku_generator

#endif // SIMPLESATSOLVER_SRC_SUDOKU_GENERATOR_GENERATOR_H_
