//
// Created by jens on 23-09-20.
//

#include "generator.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>

#include "../sat/sat_problem.h"
#include "../sudoku/encoder.h"

namespace simple_sat_solver::sudoku_generator {

Generator::Generator(solver_wrappers::ISolver *solver)
    : solver_(solver), encoder(0) {}
Generator::~Generator() { delete solver_; }
Sudoku Generator::Generate(int sub_size, int given) {
  encoder = sudoku::Encoder(sub_size);
  for (int i = 0; i < 10; i++) {
    std::cout<<"Generate sudoku" << std::endl;
    Sudoku p = GenerateUnique(sub_size);
    sat::SatProblem sat = encoder.Encode(p);
    if (!solver_->Solve(sat))
      throw "Error";
    std::vector<bool> sat_sol = solver_->GetSolution();
    Sudoku solution = encoder.Decode(sat_sol);

    int filled_cells = GetFilledCells(p);

    if (filled_cells == given) {
      return p;
    } else if (filled_cells < given) {
      if (RevealExtraCells(p, solution, given - filled_cells))
        return p;
      else
        throw "Not possible";
    } else {
      if (ReduceSolution(p, solution, filled_cells - given)) {
        //   ExhaustiveSearch(p, solution, given);
        if (GetFilledCells(p) == given) {
          return p;
        }
      } else if (GetFilledCells(p) < given) {
        throw "Error";
      } else {
        // search in related problems, might be smaller
        for (int i = 0; i < 10; i++) {

          std::cout<<"Local search" << std::endl;
          if (!SolutionIsUnique(p, solution))
            throw "Error";
          filled_cells = GetFilledCells(p);
          ExpandSolution(p, solution, sat_sol, 2);
          filled_cells = GetFilledCells(p);
          if (ReduceSolution(p, solution, filled_cells - given))
            return p;
        }
        // TODO to slow, might be able to improve the speed.
        // if (ExhaustiveSearch(p, solution, given)) {
        //  return p;
        //}
      }
    }
  }

  throw "Not possible";
}
std::vector<int> Generator::DifferenceIndex(const Sudoku &sol1,
                                            const Sudoku &sol2) const {
  if (sol1.cells.size() != sol2.cells.size())
    throw "Solutions have different sizes";
  std::vector<int> diff_indices;
  for (int i = 0; i < sol1.cells.size(); i++) {
    if (sol1.cells[i] != sol2.cells[i])
      diff_indices.push_back(i);
  }
  return diff_indices;
}
sat::SatProblem
Generator::CreateSecondSolutionProblem(const sat::SatProblem &problem,
                                       const std::vector<bool> &vector) {
  sat::SatProblem second_solution_problem = problem;
  std::vector<sat::Lit> clause;
  for (int i = 0; i < vector.size(); i++) {
    if (vector[i])
      clause.push_back(~sat::Lit(i));
  }
  second_solution_problem.AddClause(clause);
  return second_solution_problem;
}
Sudoku Generator::GenerateUnique(int sub_size) {
  int size = sub_size * sub_size;
  std::vector<int> cells(size * size, -1);
  std::srand(std::time(nullptr));
  int i = std::rand() % cells.size();
  int v = (std::rand() % size) + 1;
  cells[i] = v;

  Sudoku problem(sub_size, cells);
  sat::SatProblem s = encoder.Encode(problem);
  solver_->Solve(s);
  std::vector<bool> sat_solution = solver_->GetSolution();
  Sudoku solution = encoder.Decode(sat_solution);
  bool unique = false;
  while (!unique) {

    s = encoder.Encode(problem);
    unique = SolutionIsUnique(s, sat_solution);
    if (!unique) {
      std::vector<bool> second_sat_solution = solver_->GetSolution();
      Sudoku second_solution = encoder.Decode(second_sat_solution);
      std::vector<int> differences = DifferenceIndex(solution, second_solution);
      if (differences.empty())
        throw "Solutions are the same?";
      if (std::rand() % 2 == 0) {
        solution = second_solution;
        sat_solution = second_sat_solution;
      }
      int diff_index = std::rand() % differences.size();
      problem.cells[differences[diff_index]] =
          solution.cells[differences[diff_index]];
    }
  }

  sat::SatProblem sat_problem = encoder.Encode(problem);
  if (!solver_->Solve(sat_problem))
    throw "Error";
  auto sat_sol = solver_->GetSolution();
  if (!SolutionIsUnique(sat_problem, sat_sol)) {
    bool test = SolutionIsUnique(s, sat_solution);
    throw "Error";
  }
  return problem;
}
bool Generator::RevealExtraCells(Sudoku &problem, Sudoku solution,
                                 int reveal_cells) {
  int size = problem.sub_size * problem.sub_size;
  int max_unrevealed_cells = size * size; // upperbound not accurate
  while (reveal_cells > 0 && max_unrevealed_cells > 0) {
    int unrevealed_index = std::rand() % max_unrevealed_cells;
    max_unrevealed_cells = 0;
    for (int i = 0; i < problem.cells.size(); i++) {
      if (problem.cells[i] < 1) {
        if (max_unrevealed_cells == unrevealed_index) {
          problem.cells[i] = solution.cells[i];
          unrevealed_index = -1;
          --reveal_cells;
        } else {
          ++max_unrevealed_cells;
        }
      }
    }
  }
  return reveal_cells == 0;
}

bool Generator::ExpandSolution(Sudoku &problem, Sudoku solution,
                               std::vector<bool> sat_solution, int max_nr) {
  std::vector<int> remove_candidates;
  for (int i = 0; i < problem.cells.size(); i++) {
    if (problem.cells[i] >= 1)
      remove_candidates.push_back(i);
  }
  for (int i = 0; i < max_nr; i++) {
    int index = std::rand() % remove_candidates.size();
    problem.cells[remove_candidates[index]] = -1;
  }

  std::vector<int> add_cells;

  sat::SatProblem sat_problem = encoder.Encode(problem);
  sat::SatProblem next_solution_problem =
      CreateSecondSolutionProblem(sat_problem, sat_solution);
  for (int i = 0; i < 10; i++) {
    bool unique = !solver_->Solve(next_solution_problem);
    if (!unique) {
      auto next_sat_sol = solver_->GetSolution();
      auto next_sol = encoder.Decode(next_sat_sol);
      for (int i : DifferenceIndex(solution, next_sol))
        add_cells.push_back(i);
      next_solution_problem =
          CreateSecondSolutionProblem(next_solution_problem, sat_solution);
    } else {
      break;
    }
  }

  for (int i : add_cells) {
    problem.cells[i] = solution.cells[i];
  }
  for (int i: remove_candidates) {
    problem.cells[i] = solution.cells[i];
  }
  if (!SolutionIsUnique(problem, solution))
    throw "Error";
  return true;
}

bool Generator::ReduceSolution(Sudoku &problem, Sudoku solution,
                               int remove_givens) {
  std::vector<int> check_index_list;
  for (int i = 0; i < problem.cells.size(); i++) {
    if (problem.cells[i] >= 1)
      check_index_list.push_back(i);
  }
  for (int i = 0; i < remove_givens; i++) {
    std::vector<int> remove_from_index_list;
    bool removed = false;
    if (check_index_list.empty())
      return false;
    int start_index = rand() % check_index_list.size();
    for (int c_i = 0; c_i < check_index_list.size(); c_i++) {
     int i = check_index_list[(c_i + start_index) % check_index_list.size()];
      if (problem.cells[i] >= 1) {
        int value = problem.cells[i];
        problem.cells[i] = -1;
        remove_from_index_list.push_back(i);
        if (SolutionIsUnique(problem, solution)) {
          removed =true;
          break;
        } else {
          problem.cells[i] = solution.cells[i];
        }
      }
    }
    if (!removed)
      return false;
    std::vector<int> new_index_list;
    for (int i : check_index_list) {
      if (std::find(remove_from_index_list.begin(), remove_from_index_list.end(), i) == remove_from_index_list.end())
        new_index_list.push_back(i);
    }
    check_index_list = new_index_list;
  }
  return true;
}

bool Generator::SolutionIsUnique(sat::SatProblem problem,
                                 std::vector<bool> solution) {
  sat::SatProblem second_solution_finder =
      CreateSecondSolutionProblem(problem, solution);
  return !solver_->Solve(second_solution_finder);
}
bool Generator::SolutionIsUnique(Sudoku problem, Sudoku solution) {

  solver_->Solve(encoder.Encode(solution));
  std::vector<bool> sat_solution = solver_->GetSolution();
  return SolutionIsUnique(encoder.Encode(problem), sat_solution);
}
int Generator::GetFilledCells(Sudoku problem) {
  int filled_cells = 0;
  for (int c : problem.cells) {
    if (c >= 1)
      filled_cells++;
  }
  return filled_cells;
}
bool Generator::ExhaustiveSearch(Sudoku &problem, Sudoku solution, int given) {
  solver_->Solve(encoder.Encode(solution));
  std::vector<bool> sat_solution = solver_->GetSolution();

  problem =
      Sudoku(solution.sub_size, std::vector<int>(solution.cells.size(), -1));
  if (MinSolution(problem, solution, sat_solution, 0, given)) {
    return true;
  }
  return false;
}
bool Generator::MinSolution(Sudoku &problem, Sudoku solution,
                            std::vector<bool> sat_solution, int next_cell,
                            int unassigned_givens) {
  if (unassigned_givens < 0)
    return false;

  sat::SatProblem sat_problem = encoder.Encode(problem);
  sat::SatProblem second_sol_problem =
      CreateSecondSolutionProblem(sat_problem, sat_solution);
  bool unique = !solver_->Solve(second_sol_problem);
  if (!unique) {
    auto second_sat_sol = solver_->GetSolution();
    Sudoku second_sol = encoder.Decode(second_sat_sol);
    std::vector<int> candidates = DifferenceIndex(solution, second_sol);
    for (int c : candidates) {
      if (c < next_cell)
        continue;
      problem.cells[c] = solution.cells[c];
      if (MinSolution(problem, solution, sat_solution, c + 1,
                      unassigned_givens - 1)) {
        return true;
      }
      problem.cells[c] = -1;
    }
    return false;
  } else {
    for (int i = 0; i < problem.cells.size(); i++) {
      if (unassigned_givens == 0)
        break;
      if (problem.cells[i] == -1) {
        problem.cells[i] = solution.cells[i];
        --unassigned_givens;
      }
    }
  }

  return unassigned_givens == 0;
}
} // namespace simple_sat_solver::sudoku_generator