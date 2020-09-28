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
  CreateStartBoard(problem, 25); // size_ * size_ - hidden_cells);
  // CreateNextBoard(problem);
  for (int i = 0; i < 25; ++i) {
    CreateNextBoard(
        problem,
        i % 2);
  }
  // sat::SatProblem problem = sat::SatProblem(size_ * size_ * size_);
  // sudoku_start_indices_.push_back(0);
  AddSolvedConstraints(problem,
                       sudoku_start_indices_[sudoku_start_indices_.size() - 1]);
  std::cout << problem.GetNrVars() << "  --  " << problem.GetClauses().size()
            << std::endl;

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
void GeneratorV2::CreateNextBoard(sat::SatProblem &problem,
                                  bool advanced_reasons) {
  int start_index = problem.AddNewVars(size_ * size_ * size_);
  int start_prev = sudoku_start_indices_[sudoku_start_indices_.size() - 1];
  sudoku_start_indices_.push_back(start_index);
  AddExcludedConstraints(problem, start_prev, start_index);
  std::vector<std::vector<Lit>> reason_lits =
      IntitReasonVector(start_prev, start_index);
  if (!advanced_reasons) {
    AddUniqueConstraints(start_prev, start_index, reason_lits, problem);
  } else {
    AddHiddenSingles(start_prev, start_index, reason_lits, problem);

    AddRowGridConstraint(start_prev, start_index, reason_lits, problem);
    AddColumnGridConstraint(start_prev, start_index, reason_lits, problem);
    AddGridRowConstraint(start_prev, start_index, reason_lits, problem);
    AddGridColumnConstraint(start_prev, start_index, reason_lits, problem);
    AddPairColumn(start_prev, reason_lits, problem);
    AddPairRow(start_prev, reason_lits, problem);
  //  AddPairGrid(start_prev, reason_lits, problem);
  }
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

void GeneratorV2::AddUniqueInGroupConstraint(sat::SatProblem &problem, std::vector<std::vector<Lit>> &reasons, int start_r_index) {

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
      for (int x = 0; x < size_; ++x) {
        for (int x2 = x + 1; x2 < size_; ++x2) {
          problem.AddClause({~Lit(hidden_single_in_row),
                             ~Lit(VarIndex(prev_start_index, x, y, v)),
                             ~Lit(VarIndex(prev_start_index, x2, y, v))});
        }

        //  int reason = problem.AddNewVar();
        //  problem.AddClause({~Lit(reason), Lit(hidden_single_in_row)});
        problem.AddClause({~Lit(hidden_single_in_row),
                           Lit(VarIndex(prev_start_index, x, y, v))});

        for (int v2 = 0; v2 < size_; ++v2) {
          reasons[VarIndex(0, x, y, v2)].push_back(Lit(hidden_single_in_row));
        }
      }
    }
  }

  // COLLS
  for (int x = 0; x < size_; ++x) {
    for (int v = 0; v < size_; ++v) {
      int hidden_single_in_row = problem.AddNewVar();
      for (int y = 0; y < size_; ++y) {
        for (int y2 = y + 1; y2 < size_; ++y2) {
          problem.AddClause({~Lit(hidden_single_in_row),
                             ~Lit(VarIndex(prev_start_index, x, y, v)),
                             ~Lit(VarIndex(prev_start_index, x, y2, v))});
        }

        //        int reason = problem.AddNewVar();
        //        problem.AddClause({~Lit(reason), Lit(hidden_single_in_row)});
        problem.AddClause({~Lit(hidden_single_in_row),
                           Lit(VarIndex(prev_start_index, x, y, v))});

        for (int v2 = 0; v2 < size_; ++v2) {
          reasons[VarIndex(0, x, y, v2)].push_back(Lit(hidden_single_in_row));
        }
      }
    }
  }

  // subgrid
  for (int sub_x = 0; sub_x < sub_size_; ++sub_x) {
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
                problem.AddClause(
                    {~Lit(hidden_single_in_subgrid),
                     ~Lit(VarIndex(prev_start_index, x, y, v)),
                     ~Lit(VarIndex(prev_start_index, x2, y2, v))});
              }
            }

            //        int reason = problem.AddNewVar();
            //        problem.AddClause({~Lit(reason),
            //        Lit(hidden_single_in_subgrid)});
            problem.AddClause({~Lit(hidden_single_in_subgrid),
                               Lit(VarIndex(prev_start_index, x, y, v))});

            for (int v2 = 0; v2 < size_; ++v2) {
              reasons[VarIndex(0, x, y, v2)].push_back(
                  Lit(hidden_single_in_subgrid));
            }
          }
        }
      }
    }
  }
}
void GeneratorV2::AddRowGridConstraint(
    int prev_start_index, int start_index,
    std::vector<std::vector<sat::Lit>> &reasons, sat::SatProblem &problem) {

  for (int v = 0; v < size_; ++v) {
    for (int y = 0; y < size_; ++y) {
      for (int sub_x = 0; sub_x < sub_size_; ++sub_x) {
        int forced_in_sub_grid = problem.AddNewVar();
        int start_grid_x = sub_x * sub_size_;
        int end_grid_x = start_grid_x + sub_size_;
        for (int x = 0; x < size_; ++x) {
          if (x >= start_grid_x && x < end_grid_x)
            continue;
          problem.AddClause({~Lit(VarIndex(prev_start_index, x, y, v)),
                             ~Lit(forced_in_sub_grid)});
        }

        // problem.AddClause({~Lit(forced_in_sub_grid)});
        int start_y = y - y % sub_size_;
        for (int x = start_grid_x; x < end_grid_x; ++x) {

          for (int y2 = start_y; y2 < start_y + sub_size_; ++y2) {
            if (y2 == y)
              continue;
            reasons[VarIndex(0, x, y2, v)].push_back(Lit(forced_in_sub_grid));
          }
        }
      }
    }
  }
}
void GeneratorV2::AddColumnGridConstraint(
    int prev_start_index, int start_index,
    std::vector<std::vector<sat::Lit>> &reasons, sat::SatProblem &problem) {
  for (int v = 0; v < size_; ++v) {
    for (int x = 0; x < size_; ++x) {
      for (int sub_y = 0; sub_y < sub_size_; ++sub_y) {
        int forced_in_sub_grid = problem.AddNewVar();
        int start_grid_y = sub_y * sub_size_;
        int end_grid_y = start_grid_y + sub_size_;
        for (int y = 0; y < size_; ++y) {
          if (y >= start_grid_y && y < end_grid_y)
            continue;
          problem.AddClause({~Lit(VarIndex(prev_start_index, x, y, v)),
                             ~Lit(forced_in_sub_grid)});
        }

        // problem.AddClause({~Lit(forced_in_sub_grid)});
        int start_x = x - x % sub_size_;
        for (int y = start_grid_y; y < end_grid_y; ++y) {

          for (int x2 = start_x; x2 < start_x + sub_size_; ++x2) {
            if (x2 == x)
              continue;
            reasons[VarIndex(0, x2, y, v)].push_back(Lit(forced_in_sub_grid));
          }
        }
      }
    }
  }
}
void GeneratorV2::AddGridRowConstraint(
    int prev_start_index, int start_index,
    std::vector<std::vector<sat::Lit>> &reasons, sat::SatProblem &problem) {
  for (int v = 0; v < size_; ++v) {
    for (int sub_x = 0; sub_x < sub_size_; ++sub_x) {
      for (int sub_y = 0; sub_y < sub_size_; ++sub_y) {
        int y_start = sub_y * sub_size_;
        int y_end = y_start + sub_size_;
        int x_start = sub_x * sub_size_;
        int x_end = x_start + sub_size_;
        for (int y = y_start; y < y_end; ++y) {
          // start checking if value v is forces in current subgrid to row y
          int forced_in_sub_grid = problem.AddNewVar();
          for (int y2 = y_start; y2 < y_end; ++y2) {
            if (y == y2)
              continue;
            for (int x = x_start; x < x_end; ++x) {
              problem.AddClause({~Lit(VarIndex(prev_start_index, x, y2, v)),
                                 ~Lit(forced_in_sub_grid)});
            }
          }
          for (int x = 0; x < size_; ++x) {
            if (x >= x_start && x < x_end)
              continue;
            reasons[VarIndex(0, x, y, v)].push_back(Lit(forced_in_sub_grid));
          }
        }
      }
    }
  }
}
void GeneratorV2::AddGridColumnConstraint(
    int prev_start_index, int start_index,
    std::vector<std::vector<sat::Lit>> &reasons, sat::SatProblem &problem) {
  for (int v = 0; v < size_; ++v) {
    for (int sub_x = 0; sub_x < sub_size_; ++sub_x) {
      for (int sub_y = 0; sub_y < sub_size_; ++sub_y) {
        int y_start = sub_y * sub_size_;
        int y_end = y_start + sub_size_;
        int x_start = sub_x * sub_size_;
        int x_end = x_start + sub_size_;
        for (int x = x_start; x < x_end; ++x) {
          // start checking if value v is forces in current subgrid to row y
          int forced_in_sub_grid = problem.AddNewVar();
          for (int x2 = x_start; x2 < x_end; ++x2) {
            if (x == x2)
              continue;
            for (int y = y_start; y < y_end; ++y) {
              problem.AddClause({~Lit(VarIndex(prev_start_index, x2, y, v)),
                                 ~Lit(forced_in_sub_grid)});
            }
          }
          for (int y = 0; y < size_; ++y) {
            if (y >= y_start && y < y_end)
              continue;
            reasons[VarIndex(0, x, y, v)].push_back(Lit(forced_in_sub_grid));
          }
        }
      }
    }
  }
}
void GeneratorV2::AddPairRow(int prev_start_index,
                             std::vector<std::vector<sat::Lit>> &reasons,
                             sat::SatProblem &problem) {
  for (int y = 0; y < size_; ++y) {
    std::vector<int> pair;
    for (int v = 0; v < size_; ++v)
      pair.push_back(problem.AddNewVar());
    // select at most 2
    for (int v1 = 0; v1 < size_ - 2; ++v1) {
      for (int v2 = v1 + 1; v2 < size_ - 1; ++v2) {
        for (int v3 = v2 + 1; v3 < size_; ++v3) {
          problem.AddClause({~Lit(pair[v1]), ~Lit(pair[v2]), ~Lit(pair[v3])});
        }
      }
    }
    // check which x match the pair pattern
    std::vector<int> select;
    for (int x = 0; x < size_; ++x) {
      int select_var = problem.AddNewVar();
      select.push_back(select_var);
      for (int v = 0; v < size_; ++v) {
        problem.AddClause({Lit(pair[v]),
                           ~Lit(VarIndex(prev_start_index, x, y, v)),
                           ~Lit(select_var)});
        problem.AddClause({~Lit(pair[v]),
                           Lit(VarIndex(prev_start_index, x, y, v)),
                           ~Lit(select_var)});
      }
    }
    // check if there are two (or more) positions that match the patern
    int reason = problem.AddNewVar();
    for (int x1 = 0; x1 < size_; ++x1) {
      std::vector<Lit> at_least_one;
      at_least_one.push_back(~Lit(reason));
      for (int x2 = 0; x2 < size_; ++x2) {
        if (x1 == x2)
          continue;
        at_least_one.push_back(Lit(select[x2]));
      }
      problem.AtLeastOne(at_least_one);
    }

    for (int x = 0; x < size_; ++x) {
      for (int v = 0; v < size_; ++v) {
        int r = problem.AddNewVar();
        // for the current (x,y,v), if a pair exist, v is in that pair, but x
        // not a cell that is part of the pair.
        problem.AddClause({~Lit(r), Lit(pair[v])});
        problem.AddClause({~Lit(r), ~Lit(select[x])});
        problem.AddClause({~Lit(r), Lit(reason)});
        reasons[VarIndex(0, x, y, v)].push_back(Lit(r));
      }
    }
  }
}

void GeneratorV2::AddPairColumn(int prev_start_index,
                             std::vector<std::vector<sat::Lit>> &reasons,
                             sat::SatProblem &problem) {
  for (int x = 0; x < size_; ++x) {
    std::vector<int> pair;
    for (int v = 0; v < size_; ++v)
      pair.push_back(problem.AddNewVar());
    // select at most 2
    for (int v1 = 0; v1 < size_ - 2; ++v1) {
      for (int v2 = v1 + 1; v2 < size_ - 1; ++v2) {
        for (int v3 = v2 + 1; v3 < size_; ++v3) {
          problem.AddClause({~Lit(pair[v1]), ~Lit(pair[v2]), ~Lit(pair[v3])});
        }
      }
    }
    // check which x match the pair pattern
    std::vector<int> select;
    for (int y = 0; y < size_; ++y) {
      int select_var = problem.AddNewVar();
      select.push_back(select_var);
      for (int v = 0; v < size_; ++v) {
        problem.AddClause({Lit(pair[v]),
                           ~Lit(VarIndex(prev_start_index, x, y, v)),
                           ~Lit(select_var)});
        problem.AddClause({~Lit(pair[v]),
                           Lit(VarIndex(prev_start_index, x, y, v)),
                           ~Lit(select_var)});
      }
    }
    // check if there are two (or more) positions that match the patern
    int reason = problem.AddNewVar();
    for (int y1 = 0; y1 < size_; ++y1) {
      std::vector<Lit> at_least_one;
      at_least_one.push_back(~Lit(reason));
      for (int y2 = 0; y2 < size_; ++y2) {
        if (y1 == y2)
          continue;
        at_least_one.push_back(Lit(select[y2]));
      }
      problem.AtLeastOne(at_least_one);
    }

    for (int y = 0; y < size_; ++y) {
      for (int v = 0; v < size_; ++v) {
        int r = problem.AddNewVar();
        // for the current (x,y,v), if a pair exist, v is in that pair, but x
        // not a cell that is part of the pair.
        problem.AddClause({~Lit(r), Lit(pair[v])});
        problem.AddClause({~Lit(r), ~Lit(select[y])});
        problem.AddClause({~Lit(r), Lit(reason)});
        reasons[VarIndex(0, x, y, v)].push_back(Lit(r));
      }
    }
  }
}

void GeneratorV2::AddPairGrid(int prev_start_index,
                             std::vector<std::vector<sat::Lit>> &reasons,
                             sat::SatProblem &problem) {
  for (int sub_grid = 0; sub_grid < size_; ++sub_grid) {
    std::vector<int> pair;
    for (int v = 0; v < size_; ++v)
      pair.push_back(problem.AddNewVar());
    // select at most 2
    for (int v1 = 0; v1 < size_ - 2; ++v1) {
      for (int v2 = v1 + 1; v2 < size_ - 1; ++v2) {
        for (int v3 = v2 + 1; v3 < size_; ++v3) {
          problem.AddClause({~Lit(pair[v1]), ~Lit(pair[v2]), ~Lit(pair[v3])});
        }
      }
    }
    // check which x match the pair pattern
    std::vector<int> select;
    for (int offset_x = 0; offset_x < sub_size_; ++offset_x) {
      for (int offset_y = 0; offset_y < sub_size_; ++offset_y) {
        int sub_x = sub_grid % sub_size_;
        int sub_y = sub_grid / sub_size_;
        int x = sub_x * sub_size_ + offset_x;
        int y = sub_y * sub_size_ + offset_y;
        int select_var = problem.AddNewVar();
        select.push_back(select_var);
        for (int v = 0; v < size_; ++v) {
          problem.AddClause({Lit(pair[v]),
                             ~Lit(VarIndex(prev_start_index, x, y, v)),
                             ~Lit(select_var)});
          problem.AddClause({~Lit(pair[v]),
                             Lit(VarIndex(prev_start_index, x, y, v)),
                             ~Lit(select_var)});
        }
      }
    }
    // check if there are two (or more) positions that match the patern
    int reason = problem.AddNewVar();
    for (int x1 = 0; x1 < size_; ++x1) {
      std::vector<Lit> at_least_one;
      at_least_one.push_back(~Lit(reason));
      for (int x2 = 0; x2 < size_; ++x2) {
        if (x1 == x2)
          continue;
        at_least_one.push_back(Lit(select[x2]));
      }
      problem.AtLeastOne(at_least_one);
    }

    for (int offset_x = 0; offset_x < sub_size_; ++offset_x) {
      for (int offset_y = 0; offset_y < sub_size_; ++offset_y) {
        int sub_x = sub_grid % sub_size_;
        int sub_y = sub_grid / sub_size_;
        int x = sub_x * sub_size_ + offset_x;
        int y = sub_y * sub_size_ + offset_y;
        for (int v = 0; v < size_; ++v) {
          int r = problem.AddNewVar();
          // for the current (x,y,v), if a pair exist, v is in that pair, but x
          // not a cell that is part of the pair.
          problem.AddClause({~Lit(r), Lit(pair[v])});
          problem.AddClause({~Lit(r), ~Lit(select[sub_y * sub_size_ + sub_x])});
          problem.AddClause({~Lit(r), Lit(reason)});
          reasons[VarIndex(0, x, y, v)].push_back(Lit(r));
        }
      }
    }
  }
}
std::vector<int> GeneratorV2::GetRowCells(int row_index) {
  std::vector<int> res;
  for (int x = 0; x < size_; ++x)
    res.push_back(CellIndex(x,row_index));
  return res;
}
std::vector<int> GeneratorV2::GetColumnCells(int column_index) {
  std::vector<int> res;
  for (int y = 0; y < size_; ++y)
    res.push_back(CellIndex(column_index, y);
  return res;
}
std::vector<int> GeneratorV2::GetSubgridCells(int subgrid_index) {
  std::vector<int> res;
  int base_x = subgrid_index % sub_size_;
  int base_y = subgrid_index % sub_size_;
  for (int y = 0; y < sub_size_; ++y) {
    for (int x = 0; x < sub_size_; ++x) {
      res.push_back(CellIndex(base_x + x, base_y + y));
    }
  }
  return res;
}

} // namespace simple_sat_solver::sudoku_generator