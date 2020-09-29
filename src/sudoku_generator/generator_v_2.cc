//
// Created by jens on 24-09-20.
//

#include "generator_v_2.h"
#include <algorithm>
#include <iostream>
namespace simple_sat_solver::sudoku_generator {

using sat::Lit;
sudoku::Sudoku GeneratorV2::Generate(int revealed) {
  sat::SatProblem problem = sat::SatProblem(0);
  int hidden_cells = size_ * size_ - revealed;

  CreateStartState(problem, revealed); // size_ * size_ - hidden_cells);
  for (int i = 0; i < hidden_cells / 3; ++i) {
    CreateNextState(problem, i % 2);
  }
  AddSolvedConstraints(problem,
                       sudoku_start_indices_[sudoku_start_indices_.size() - 1]);

  std::cout << problem.GetNrVars() << "  --  " << problem.GetClauses().size()
            << std::endl;

  if (solver_->Solve(problem)) {
    auto result = solver_->GetSolution();
    sudoku::Sudoku sudoku = SatToStartSudoku(result, 0);
    return sudoku;
  }
  return sudoku::Sudoku();
}
GeneratorV2::~GeneratorV2() { delete solver_; }

void GeneratorV2::CreateNextState(sat::SatProblem &problem,
                                  bool advanced_reasons) {
  int start_index = problem.AddNewVars(size_ * size_ * size_);
  int start_prev = sudoku_start_indices_[sudoku_start_indices_.size() - 1];
  sudoku_start_indices_.push_back(start_index);

  AddExcludedConstraints(problem, start_prev, start_index);

  // reason_lits keeps for all c,v pairs track of the reason that can set it to
  // false. if all the reasons are added the following clause is added:
  // ~prev(c,v) V current(c,v) V R1 V R2 V R3 ...
  // where Ri is stored in the reason_lits.
  std::vector<std::vector<Lit>> reason_lits =
      IntitReasonVector(start_prev, start_index);
  // do not allways add all possible reasoning steps, problem becomes to big.
  if (!advanced_reasons) {
    AddUniqueConstraints(start_prev, reason_lits, problem);
  } else {
    AddHiddenSingles(start_prev, reason_lits, problem);
    AddRowGridConstraint(start_prev, reason_lits, problem);
    AddColumnGridConstraint(start_prev, reason_lits, problem);
    // TODO fix bug in pairs functions
    //        AddPairs(start_index, reason_lits, problem);
  }

  // add the clauses that allow a c,v to become false based on the constructed
  // reasons.
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
void GeneratorV2::AddUniqueConstraints(int prev_start_index,
                                       std::vector<std::vector<Lit>> &reasons,
                                       sat::SatProblem &problem) {
  int start_r_index = problem.GetNrVars();
  // check which (c,v) are the unique values in their c.
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

  for (int x = 0; x < size_; ++x)
    AddUniqueInGroupConstraint(problem, reasons, GetRowCells(x), start_r_index);
  for (int y = 0; y < size_; ++y)
    AddUniqueInGroupConstraint(problem, reasons, GetColumnCells(y),
                               start_r_index);
  for (int g = 0; g < size_; ++g)
    AddUniqueInGroupConstraint(problem, reasons, GetSubgridCells(g),
                               start_r_index);
}

void GeneratorV2::AddUniqueInGroupConstraint(
    sat::SatProblem &problem, std::vector<std::vector<Lit>> &reasons,
    std::vector<int> cell_indices, int start_r_index) {
  for (int c1 : cell_indices) {
    for (int v = 0; v < size_; v++) {
      for (int c2 : cell_indices) {
        if (c1 == c2)
          continue;
        reasons[VarIndex(0, c1, v)].push_back(VarIndex(start_r_index, c2, v));
      }
    }
  }
}

void GeneratorV2::AddSolvedConstraints(sat::SatProblem &problem,
                                       int start_index) {

  CreateSolvedCellConstraints(problem, start_index);
  for (int x = 0; x < size_; ++x)
    CreateSolvedGroupConstraint(problem, GetRowCells(x), start_index);
  for (int y = 0; y < size_; ++y)
    CreateSolvedGroupConstraint(problem, GetColumnCells(y), start_index);
  for (int g = 0; g < size_; ++g)
    CreateSolvedGroupConstraint(problem, GetSubgridCells(g), start_index);
}

void GeneratorV2::CreateSolvedCellConstraints(sat::SatProblem &problem,
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

void GeneratorV2::CreateSolvedGroupConstraint(sat::SatProblem &problem,
                                              std::vector<int> cell_indices,
                                              int start_index) {
  for (int v = 0; v < size_; v++) {
    std::vector<sat::Lit> exactly_one;
    exactly_one.reserve(size_);
    for (int c : cell_indices) {
      exactly_one.emplace_back(VarIndex(start_index, c, v), false);
    }
    problem.ExactlyOne(exactly_one);
  }
}

void GeneratorV2::CreateStartState(sat::SatProblem &problem,
                                   int revealed_cells) {

  if (revealed_cells < 0 || revealed_cells > size_ * size_)
    throw "Not possible";
  int start_index = problem.AddNewVars(size_ * size_ * size_);
  sudoku_start_indices_.push_back(start_index);
  // cells should have either one possible value, or all values are possible.
  // keep track of the cells that can have only one value
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

  // specify that there must be exactly revealed_cells given cells.
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
                                             int state_index) {
  std::vector<int> cells(size_ * size_, -1);
  int start_index = sudoku_start_indices_[state_index];
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

void GeneratorV2::AddHiddenSingles(int prev_start_index,
                                   std::vector<int> cell_indices,
                                   std::vector<std::vector<sat::Lit>> &reasons,
                                   sat::SatProblem &problem) {
  for (int v = 0; v < size_; ++v) {
    int hidden_single = problem.AddNewVar();
    for (int c1 = 0; c1 < cell_indices.size(); ++c1) {
      for (int c2 = c1 + 1; c2 < cell_indices.size(); ++c2) {
        // test if there v is a hidden single in the group
        problem.AddClause({~Lit(hidden_single),
                           ~Lit(VarIndex(prev_start_index, c1, v)),
                           ~Lit(VarIndex(prev_start_index, c2, v))});
      }

      // test if c if the hidden single
      int reason = problem.AddNewVar();
      problem.Implies(reason, hidden_single);
      problem.AddClause(
          {~Lit(reason), Lit(VarIndex(prev_start_index, c1, v))});

      for (int v2 = 0; v2 < size_; ++v2) {
        reasons[VarIndex(0, c1, v2)].push_back(Lit(reason));
      }
    }
  }
}
void GeneratorV2::AddHiddenSingles(int prev_start_index,
                                   std::vector<std::vector<Lit>> &reasons,
                                   sat::SatProblem &problem) {
  for (int y = 0; y < size_; ++y)
    AddHiddenSingles(prev_start_index, GetRowCells(y), reasons, problem);
  for (int x = 0; x < size_; ++x)
    AddHiddenSingles(prev_start_index, GetColumnCells(x), reasons, problem);
  for (int g = 0; g < size_; ++g)
    AddHiddenSingles(prev_start_index, GetSubgridCells(g), reasons, problem);
}

void GeneratorV2::AddGroupConstrainsGroup(
    int prev_start_index, std::vector<int> cause, std::vector<int> effect,
    std::vector<std::vector<Lit>> &reasons, sat::SatProblem &problem) {

  std::sort(cause.begin(), cause.end());
  std::sort(effect.begin(), effect.end());

  std::vector<int> cause_reduced;
  std::vector<int> effect_reduced;
  // not need to sort since it already sorted
  std::set_difference(cause.begin(), cause.end(), effect.begin(), effect.end(),
                      std::inserter(cause_reduced, cause_reduced.begin()));

  std::set_difference(effect.begin(), effect.end(), cause.begin(), cause.end(),
                      std::inserter(effect_reduced, effect_reduced.begin()));

  for (int v = 0; v < size_; ++v) {
    int forced_in_sub_grid = problem.AddNewVar();
    for (int c : cause_reduced) {
      problem.AddClause(
          {~Lit(VarIndex(prev_start_index, c, v)), ~Lit(forced_in_sub_grid)});
    }

    for (int e : effect_reduced) {
      reasons[VarIndex(0, e, v)].push_back(Lit(forced_in_sub_grid));
    }
  }
}
void GeneratorV2::AddRowGridConstraint(
    int prev_start_index, std::vector<std::vector<sat::Lit>> &reasons,
    sat::SatProblem &problem) {
  // loop over the intersecting rows and subgrids.
  for (int y = 0; y < size_; ++y) {
    int group_start = (y / sub_size_) * sub_size_;
    int group_step = 1;
    int group_end = group_start + group_step * sub_size_;
    for (int g = group_start; g < group_end; g += group_step) {
      AddGroupConstrainsGroup(prev_start_index, GetSubgridCells(g),
                              GetRowCells(y), reasons, problem);
      AddGroupConstrainsGroup(prev_start_index, GetRowCells(y),
                              GetSubgridCells(g), reasons, problem);
    }
  }
}
void GeneratorV2::AddColumnGridConstraint(
    int prev_start_index, std::vector<std::vector<sat::Lit>> &reasons,
    sat::SatProblem &problem) {
  // loop over the intersecting columns and subgrids.
  for (int x = 0; x < size_; ++x) {
    int group_start = (x / sub_size_);
    int group_step = sub_size_;
    int group_end = group_start + group_step * sub_size_;
    for (int g = group_start; g < group_end; g += group_step) {
      AddGroupConstrainsGroup(prev_start_index, GetSubgridCells(g),
                              GetRowCells(x), reasons, problem);
      AddGroupConstrainsGroup(prev_start_index, GetRowCells(x),
                              GetSubgridCells(g), reasons, problem);
    }
  }
}

void GeneratorV2::AddPairs(int prev_start_index,
                           std::vector<std::vector<Lit>> &reasons,
                           sat::SatProblem &problem) {
  for (int y = 0; y < size_; ++y)
    AddPairGroup(prev_start_index, GetRowCells(y), reasons, problem);
  for (int x = 0; x < size_; ++x)
    AddPairGroup(prev_start_index, GetColumnCells(x), reasons, problem);
  for (int g = 0; g < size_; ++g)
    AddPairGroup(prev_start_index, GetSubgridCells(g), reasons, problem);
}

void GeneratorV2::AddPairGroup(int prev_start_index,
                               std::vector<int> cell_indices,
                               std::vector<std::vector<sat::Lit>> &reasons,
                               sat::SatProblem &problem) {
  // todo fix bug
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
  for (int c : cell_indices) {
    int select_var = problem.AddNewVar();
    select.push_back(select_var);
    for (int v = 0; v < size_; ++v) {
      problem.AddClause({Lit(pair[v]), ~Lit(VarIndex(prev_start_index, c, v)),
                         ~Lit(select_var)});
      problem.AddClause({~Lit(pair[v]), Lit(VarIndex(prev_start_index, c, v)),
                         ~Lit(select_var)});
    }
  }
  // check if there are two (or more) positions that match the pattern
  int reason = problem.AddNewVar();
  for (int c1 : cell_indices) {
    std::vector<Lit> at_least_one;
    at_least_one.push_back(~Lit(reason));
    for (int i = 0; i < cell_indices.size(); ++i) {
      if (c1 == cell_indices[i])
        continue;
      at_least_one.push_back(Lit(select[i]));
    }
    problem.AtLeastOne(at_least_one);
  }

  for (int i = 0; i < cell_indices.size(); ++i) {
    for (int v = 0; v < size_; ++v) {
      int r = problem.AddNewVar();
      // for the current (x,y,v), if a pair exist, v is in that pair, but x
      // not a cell that is part of the pair.
      problem.AddClause({~Lit(r), Lit(pair[v])});
      problem.AddClause({~Lit(r), ~Lit(select[i])});
      problem.AddClause({~Lit(r), Lit(reason)});
      reasons[VarIndex(0, cell_indices[i], v)].push_back(Lit(r));
    }
  }
}

std::vector<int> GeneratorV2::GetRowCells(int row_index) {
  std::vector<int> res;
  for (int x = 0; x < size_; ++x)
    res.push_back(CellIndex(x, row_index));
  return res;
}
std::vector<int> GeneratorV2::GetColumnCells(int column_index) {
  std::vector<int> res;
  for (int y = 0; y < size_; ++y)
    res.push_back(CellIndex(column_index, y));
  return res;
}
std::vector<int> GeneratorV2::GetSubgridCells(int subgrid_index) {
  std::vector<int> res;
  int base_x = (subgrid_index % sub_size_) * sub_size_;
  int base_y = (subgrid_index / sub_size_) * sub_size_;
  for (int y = 0; y < sub_size_; ++y) {
    for (int x = 0; x < sub_size_; ++x) {
      res.push_back(CellIndex(base_x + x, base_y + y));
    }
  }
  return res;
}

} // namespace simple_sat_solver::sudoku_generator