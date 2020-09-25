//
// Created by jens on 24-09-20.
//

#include "generator_v_2.h"
#include <iostream>
namespace simple_sat_solver::sudoku_generator {

using sat::Lit;
sudoku::Sudoku GeneratorV2::Generate() {
  SudokuDomain s(sub_size_);
  for (int i = 0; i < size_; i++) {
    if (i == 5)
      continue;
    s.domains[0][i] = false;
  }
  sat::SatProblem sat = SudokuDomainToSat(s);
  return sudoku::Sudoku();
}
sat::SatProblem GeneratorV2::SudokuDomainToSat(SudokuDomain domain) {

  sat::SatProblem problem = sat::SatProblem(0);
  CreateStartBoard(problem, domain);
  CreateNextBoard(problem);
  if (solver_->Solve(problem)) {
    std::vector<bool> solution = solver_->GetSolution();

    SudokuDomain s = SatToSudokuDomain(solution, 0);
    SudokuDomain s1 = SatToSudokuDomain(solution, 1);
    int index = sudoku_start_indices_[1] + size_ * size_ * size_;
    int index_5 = index + 5;
    bool res1 = solution[index_5];
    int test = 0;
  }

  return sat::SatProblem(0);
}
GeneratorV2::~GeneratorV2() { delete solver_; }
SudokuDomain GeneratorV2::SatToSudokuDomain(std::vector<bool> sat_solution,
                                            int sudoku_index) {
  SudokuDomain domain(sub_size_);
  int start_index = sudoku_start_indices_[sudoku_index];
  for (int c = 0; c < domain.domains.size(); c++) {
    for (int v = 0; v < size_; v++) {
      domain.domains[c][v] = sat_solution[VarIndex(start_index, c, v)];
    }
  }
  return domain;
}
void GeneratorV2::CreateStartBoard(sat::SatProblem &problem,
                                   const SudokuDomain &domain) {
  int start_index = problem.AddNewVars(size_ * size_ * size_);
  sudoku_start_indices_.push_back(start_index);
  for (int c = 0; c < domain.domains.size(); c++) {
    for (int v = 0; v < size_; v++) {
      problem.AddClause(
          {Lit(VarIndex(start_index, c, v), !domain.domains[c][v])});
    }
  }
}
void GeneratorV2::CreateNextBoard(sat::SatProblem &problem) {
  int start_index = problem.AddNewVars(size_ * size_ * size_);
  int start_prev = sudoku_start_indices_[sudoku_start_indices_.size() - 1];
  sudoku_start_indices_.push_back(start_index);
  AddExcludedConstraints(problem, start_prev, start_index);
  std::vector<std::vector<Lit>> reason_lits =
      IntitReasonVector(start_prev, start_index);
  AddUniqueConstraints(start_prev, start_index, reason_lits, problem);
  problem.AddClause({~Lit(VarIndex(start_index,1,1,5))});
  AddIncludedDomainConstraints(problem, reason_lits);
}
void GeneratorV2::AddExcludedConstraints(sat::SatProblem &problem,
                                         int prev_start_index,
                                         int start_index) {
  for (int c = 0; c < size_ * size_; c++) {
    for (int v = 0; v < size_; v++) {
      problem.Implies(~Lit(VarIndex(prev_start_index, c, v)),
                      ~Lit(VarIndex(start_index, c, v)));
    }
  }
}
void GeneratorV2::AddIncludedDomainConstraints(
    sat::SatProblem &problem, std::vector<std::vector<sat::Lit>> &reasons) {
  for (int c = 0; c < size_ * size_; c++) {
    for (int v = 0; v < size_; v++) {
      problem.AddClause(reasons[VarIndex(0, c, v)]);
    }
  }
}
std::vector<std::vector<Lit>>
GeneratorV2::IntitReasonVector(int prev_start_index, int start_index) {
  std::vector<std::vector<Lit>> reasons(size_ * size_ * size_,
                                        std::vector<Lit>());
  for (int c = 0; c < size_ * size_; c++) {
    for (int v = 0; v < size_; v++) {
      reasons[VarIndex(0, c, v)].push_back(Lit(VarIndex(start_index, c, v)));
      reasons[VarIndex(0, c, v)].push_back(
          ~Lit(VarIndex(prev_start_index, c, v)));
    }
  }
  return reasons;
}
void GeneratorV2::AddUniqueConstraints(int prev_start_index, int start_index,
                                       std::vector<std::vector<Lit>> &reasons,
                                       sat::SatProblem &problem) {
  int start_r_index = problem.GetNrVars();
  for (int c = 0; c < size_ * size_; c++) {
    for (int v = 0; v < size_; v++) {
      int r = problem.AddNewVar();
      for (int v2 = 0; v2 < size_; v2++) {
        if (v2 == v) {
          problem.AddClause({Lit(VarIndex(prev_start_index, c, v2)), ~Lit(r)});
        } else {
          problem.AddClause({~Lit(VarIndex(prev_start_index, c, v2)), ~Lit(r)});
        }
      }
    }
  }

  AddUniqueRowConstraint(problem, reasons, start_r_index);
  AddUniqueColumnConstraint(problem, reasons, start_r_index);
  AddUniqueSubGridConstraint(problem, reasons, start_r_index);
}
void GeneratorV2::AddUniqueColumnConstraint(
    sat::SatProblem &problem, std::vector<std::vector<Lit>> &reasons,
    int start_r_index) {
  for (int x = 0; x < size_; x++) {
    for (int y = 0; y < size_; y++) {
      for (int v = 0; v < size_; v++) {
        for (int y2 = 0; y2 < size_; y2++) {
          if (y == y2)
            continue;
          reasons[VarIndex(0, x, y, v)].push_back(
              VarIndex(start_r_index, x, y2, v));
        }
      }
    }
  }
}
void GeneratorV2::AddUniqueSubGridConstraint(
    sat::SatProblem &problem, std::vector<std::vector<Lit>> &reasons,
    int start_r_index) {
  // TODO hide some of the for tower
  for (int sub_x = 0; sub_x < sub_size_; sub_x++) {
    for (int sub_y = 0; sub_y < sub_size_; sub_y++) {
      for (int v = 0; v < size_; ++v) {

        for (int offset_x = 0; offset_x < sub_size_; offset_x++) {
          for (int offset_y = 0; offset_y < sub_size_; offset_y++) {
            int x = sub_x * sub_size_ + offset_x;
            int y = sub_y * sub_size_ + offset_y;

            for (int offset_x2 = 0; offset_x2 < sub_size_; offset_x2++) {
              for (int offset_y2 = 0; offset_y2 < sub_size_; ++offset_y2) {
                int x2 = sub_x * sub_size_ + offset_x2;
                int y2 = sub_y * sub_size_ + offset_y2;
                if (x == x2 && y == y2)
                  continue;

                reasons[VarIndex(0, x, y, v)].push_back(
                    VarIndex(start_r_index, x2, y2, v));
              }
            }
          }
        }
      }
    }
  }
}
void GeneratorV2::AddUniqueRowConstraint(sat::SatProblem &problem,
                                         std::vector<std::vector<Lit>> &reasons,
                                         int start_r_index) {
  for (int x = 0; x < size_; x++) {
    for (int y = 0; y < size_; y++) {
      for (int v = 0; v < size_; v++) {
        for (int x2 = 0; x2 < size_; x2++) {
          if (x == x2)
            continue;
          reasons[VarIndex(0, x, y, v)].push_back(
              VarIndex(start_r_index, x2, y, v));
        }
      }
    }
  }
}
} // namespace simple_sat_solver::sudoku_generator