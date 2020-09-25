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
  if (solver_->Solve(sat)) {
    auto result = solver_->GetSolution();
    sudoku::Sudoku sudoku = SatToStartSudoku(result, 0);
    return sudoku;
  }
  return sudoku::Sudoku();
}
sat::SatProblem GeneratorV2::SudokuDomainToSat(SudokuDomain domain) {

  sat::SatProblem problem = sat::SatProblem(0);
  // CreateStartBoard(problem, domain);
  int hidden_cells = 50;
  CreateStartBoard(problem, 22); // size_ * size_ - hidden_cells);
  //CreateNextBoard(problem);
  for (int i = 0; i < 25; ++i) {
    CreateNextBoard(problem, i  %2 );//i == 5 || i == 8 || i == 2 || i == 12 || i ==18);//i %2);
  }
  // sat::SatProblem problem = sat::SatProblem(size_ * size_ * size_);
  // sudoku_start_indices_.push_back(0);
  AddSolvedConstraints(problem,
                       sudoku_start_indices_[sudoku_start_indices_.size() - 1]);
//  if (solver_->Solve(problem)) {
//    std::vector<bool> solution = solver_->GetSolution();
//
//    SudokuDomain s = SatToSudokuDomain(solution, 0);
//    SudokuDomain s1 = SatToSudokuDomain(solution, 1);
//    int index = sudoku_start_indices_[1] + size_ * size_ * size_;
//    int index_5 = index + 5;
//    bool res1 = solution[index_5];
//    int test = 0;
//  }
  std::cout << problem.GetNrVars() << "  --  " << problem.GetClauses().size() << std::endl;

  return problem;
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
void GeneratorV2::CreateNextBoard(sat::SatProblem &problem, bool advanced_reasons) {
  int start_index = problem.AddNewVars(size_ * size_ * size_);
  int start_prev = sudoku_start_indices_[sudoku_start_indices_.size() - 1];
  sudoku_start_indices_.push_back(start_index);
  AddExcludedConstraints(problem, start_prev, start_index);
  std::vector<std::vector<Lit>> reason_lits =
      IntitReasonVector(start_prev, start_index);
  if (!advanced_reasons)
   AddUniqueConstraints(start_prev, start_index, reason_lits, problem);
 else
  AddHiddenSingles(start_prev, start_index, reason_lits, problem);
//  int r= problem.AddNewVar();
//  for (int i = 0; i < size_; ++i) {
//    reason_lits[i].push_back(Lit(r));
//  }
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
void GeneratorV2::AddSolvedConstraints(sat::SatProblem &problem,
                                       int start_index) {

  CreateCellConstraints(problem, start_index);
  CreateRowConstraints(problem, start_index);
  CreateColumnConstraints(problem, start_index);
  CreateSubGridConstraints(problem, start_index);
}

void GeneratorV2::CreateCellConstraints(sat::SatProblem &problem,
                                        int start_index) {
  for (int x = 0; x < size_; x++) {
    for (int y = 0; y < size_; y++) {
      std::vector<sat::Lit> exactly_one;
      exactly_one.reserve(size_);
      for (int v = 0; v < size_; v++) {
        exactly_one.emplace_back(VarIndex(start_index, x, y, v), false);
      }
      problem.ExactlyOne(exactly_one);
    }
  }
}
void GeneratorV2::CreateRowConstraints(sat::SatProblem &problem,
                                       int start_index) {
  for (int x = 0; x < size_; x++) {
    for (int v = 0; v < size_; v++) {
      std::vector<sat::Lit> exactly_one;
      exactly_one.reserve(size_);
      for (int y = 0; y < size_; y++) {
        exactly_one.emplace_back(VarIndex(start_index, x, y, v), false);
      }
      problem.ExactlyOne(exactly_one);
    }
  }
}
void GeneratorV2::CreateColumnConstraints(sat::SatProblem &problem,
                                          int start_index) {
  for (int y = 0; y < size_; y++) {
    for (int v = 0; v < size_; v++) {
      std::vector<sat::Lit> exactly_one;
      exactly_one.reserve(size_);
      for (int x = 0; x < size_; x++) {
        exactly_one.emplace_back(VarIndex(start_index, x, y, v), false);
      }
      problem.ExactlyOne(exactly_one);
    }
  }
}
void GeneratorV2::CreateSubGridConstraints(sat::SatProblem &problem,
                                           int start_index) {
  for (int v = 0; v < size_; v++) {
    for (int sub_x = 0; sub_x < sub_size_; sub_x++) {
      for (int sub_y = 0; sub_y < sub_size_; sub_y++) {
        std::vector<sat::Lit> exactly_one;
        exactly_one.reserve(size_);
        for (int x = 0; x < sub_size_; x++) {
          for (int y = 0; y < sub_size_; y++) {
            exactly_one.emplace_back(VarIndex(start_index,
                                              x + sub_x * sub_size_,
                                              y + sub_y * sub_size_, v),
                                     false);
          }
        }
        problem.ExactlyOne(exactly_one);
      }
    }
  }
}
void GeneratorV2::CreateStartBoard(sat::SatProblem &problem,
                                   int revealed_cells) {

  if (revealed_cells < 0 || revealed_cells > size_ * size_)
    throw "Not possible";
  int start_index = problem.AddNewVars(size_ * size_ * size_);
  sudoku_start_indices_.push_back(start_index);
  int unique_start_index = problem.AddNewVars(size_ * size_);
  for (int c = 0; c < size_ * size_; c++) {
    std::vector<Lit> at_least_one;
    for (int v = 0; v < size_; v++) {
      at_least_one.push_back(Lit(VarIndex(0, c, v)));
      problem.AddClause({Lit(VarIndex(0, c, v)), Lit(unique_start_index + c)});
      for (int v2 = 0; v2 < size_; v2++) {
        if (v2 == v)
          continue;
        problem.AddClause({~Lit(VarIndex(0, c, v)), ~Lit(VarIndex(0, c, v2)),
                           ~Lit(unique_start_index + c)});
      }
    }
    problem.AtLeastOne(at_least_one);
  }

  std::vector<Lit> unique_flags;
  std::vector<Lit> not_unique_flags;
  for (int c = 0; c < size_ * size_; c++) {
    unique_flags.push_back(Lit(unique_start_index + c));
    not_unique_flags.push_back(~Lit(unique_start_index + c));
  }

  problem.AtMostK(revealed_cells, unique_flags);
  problem.AtMostK(size_ * size_ - revealed_cells, not_unique_flags);
}
sudoku::Sudoku GeneratorV2::SatToStartSudoku(std::vector<bool> sat_solution,
                                             int board_index) {
  std::vector<int> cells(size_ * size_, -1);
  int start_index = sudoku_start_indices_[board_index];
  for (int c = 0; c < size_ * size_; ++c) {
    bool set = false;
    for (int v = 0; v < size_; ++v) {
      int index = VarIndex(start_index, c, v);
      if (sat_solution[index]) {
        if (set) {
          cells[c] = -1;
        } else {
          cells[c] = v + 1;
          set = true;
        }
      }
    }
  }
  return sudoku::Sudoku(sub_size_, cells);
}
void GeneratorV2::AddHiddenSingles(int prev_start_index, int start_index,
                                   std::vector<std::vector<Lit>> &reasons,
                                   sat::SatProblem &problem) {
  // ROWS
  for (int y = 0; y < size_; ++y) {
    for (int v = 0; v < size_; ++v) {
      int hidden_single_in_row = problem.AddNewVar();
      for (int x = 0; x < size_ ; ++x) {
        for (int x2 = x + 1; x2 < size_; ++x2) {
          problem.AddClause({~Lit(hidden_single_in_row),
                             ~Lit(VarIndex(prev_start_index, x, y, v)),
                             ~Lit(VarIndex(prev_start_index, x2, y, v))});
        }

        int reason = problem.AddNewVar();
        problem.AddClause({~Lit(reason), Lit(hidden_single_in_row)});
        problem.AddClause(
            {~Lit(reason), Lit(VarIndex(prev_start_index, x, y, v))});

        for (int v2 = 0; v2 < size_; ++v2) {
          reasons[VarIndex(0, x, y, v2)].push_back(Lit(reason));
        }
      }
    }
  }

  // COLLS
  for (int x = 0; x < size_; ++x) {
    for (int v = 0; v < size_; ++v) {
      int hidden_single_in_row = problem.AddNewVar();
      for (int y = 0; y < size_ ; ++y) {
        for (int y2 = y + 1; y2 < size_; ++y2) {
          problem.AddClause({~Lit(hidden_single_in_row),
                             ~Lit(VarIndex(prev_start_index, x, y, v)),
                             ~Lit(VarIndex(prev_start_index, x, y2, v))});
        }

        int reason = problem.AddNewVar();
        problem.AddClause({~Lit(reason), Lit(hidden_single_in_row)});
        problem.AddClause(
            {~Lit(reason), Lit(VarIndex(prev_start_index, x, y, v))});

        for (int v2 = 0; v2 < size_; ++v2) {
          reasons[VarIndex(0, x, y, v2)].push_back(Lit(reason));
        }
      }
    }
  }


  // subgrid
  for (int sub_x =0; sub_x < sub_size_; ++sub_x) {
    for (int sub_y = 0; sub_y < sub_size_; ++sub_y) {
    for (int v = 0; v < size_; ++v) {
      int hidden_single_in_subgrid = problem.AddNewVar();
      for (int offset_x = 0; offset_x < sub_size_; ++offset_x) {
        for (int offset_y = 0; offset_y < sub_size_; ++offset_y) {
          int x = sub_x * sub_size_ + offset_x;
          int y = sub_y * sub_size_ + offset_y;
        for (int offset_x2 = 0; offset_x2 < sub_size_; ++offset_x2) {
            for (int offset_y2 = 0; offset_y2 < sub_size_; ++offset_y2) {
              int x2 = sub_x * sub_size_ + offset_x2;
              int y2 = sub_y * sub_size_ + offset_y2;
              if (y2 < y || y2 == y && x2 < x)
                continue;
              problem.AddClause({~Lit(hidden_single_in_subgrid),
                                 ~Lit(VarIndex(prev_start_index, x, y, v)),
                                 ~Lit(VarIndex(prev_start_index, x2, y2, v))});
            }
          }

        int reason = problem.AddNewVar();
        problem.AddClause({~Lit(reason), Lit(hidden_single_in_subgrid)});
        problem.AddClause(
            {~Lit(reason), Lit(VarIndex(prev_start_index, x, y, v))});

        for (int v2 = 0; v2 < size_; ++v2) {
          reasons[VarIndex(0, x, y, v2)].push_back(Lit(reason));
        }
      }
    }}}
  }
}
} // namespace simple_sat_solver::sudoku_generator