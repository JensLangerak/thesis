//
// Created by jens on 18-09-20.
//

#include "sat_problem.h"
#include <algorithm>
namespace simple_sat_solver::sat {
void SatProblem::AddClause(const std::vector<Lit> &lits) {
  if (!std::all_of(lits.begin(), lits.end(),
                   [this](Lit l) { return l.x >= 0 && l.x < this->nr_vars_; }))
    throw "Illegal id";
  clauses_.push_back(lits);
}
void SatProblem::AtMostOne(const std::vector<Lit> &lits) {
  for (int i = 0; i < ((int)lits.size()) - 1; i++) {
    for (int j = i + 1; j < lits.size(); j++)
      clauses_.push_back({~lits[i], ~lits[j]});
  }
}
void SatProblem::ExactlyOne(const std::vector<Lit> &lits) {
  AtLeastOne(lits);
  AtMostOne(lits);
}
bool SatProblem::TestLit(const Lit &l, const std::vector<bool> &vars) {
  return (!l.complement) == vars[l.x];
}
bool SatProblem::TestClause(const std::vector<Lit> &c,
                            const std::vector<bool> &vars) {
  return std::any_of(c.begin(), c.end(),
                     [vars](const Lit &l) { return TestLit(l, vars); });
}
bool SatProblem::TestAssignment(const std::vector<bool> &vars) const {
  if (vars.size() != nr_vars_)
    throw "Number of vars differ";

  return std::all_of(
      clauses_.begin(), clauses_.end(),
      [vars](const std::vector<Lit> &c) { return TestClause(c, vars); });
}
void SatProblem::Implies(const Lit &antecedent, const Lit &consequent) {
  clauses_.push_back({~antecedent, consequent});
}
void SatProblem::AtMostK(const int k, const std::vector<Lit> &lits) {
  if (k < 0)
    throw "k should be non negative";
  if (lits.size() <= k) // always true, so nothing to encode
    return;
  if (k == 0) {
    None(lits);
    return;
  }
  // encodes using sequential encoding
  // introduces the vars s_i_j. s_i_j is true if sum(x_[0..i]) > j
  // This leads to the following rules (here i,j >0):
  // s_0_0 = x_0 -> s_0_0 V ~x_0
  // s_0_j = F -> ~s_0_j
  // s_i_0 = max(s_i-i_0, x_i)   -> ~s_i-1_0 V s_i_0     s_i_0 V ~x_i
  // s_i_j = max(s_i-1_j, s_i-1_j-1 + x_i)
  //       -> ~s_i-1_j V s_i_j      ~s_i-1_j-1 ~x_i s_i_j
  // Force the at most: ~s_i-1_k-1 V ~x_i

  const int base_s = nr_vars_;
  const int n = lits.size();
  nr_vars_ += n * k;

  auto s_index = [base_s, n, k](int lit_index, int sum_digit) {
    return base_s + lit_index * k + sum_digit;
  };

  // s_0_0
  clauses_.push_back({~lits[0], Lit(s_index(0, 0), false)});
  // s_0_j
  for (int j = 1; j < k; j++) {
    clauses_.push_back({Lit(s_index(0, j), true)});
  }
  // s_i_0
  for (int i = 1; i < n; i++) {
    clauses_.push_back(
        {Lit(s_index(i - 1, 0), true), Lit(s_index(i, 0), false)});
    clauses_.push_back({Lit(s_index(i, 0), false), ~lits[i]});
  }
  // s_i_j
  for (int i = 1; i < n; i++) {
    for (int j = 1; j < k; j++) {
      clauses_.push_back(
          {Lit(s_index(i - 1, j), true), Lit(s_index(i, j), false)});
      clauses_.push_back({Lit(s_index(i - 1, j - 1), true), ~lits[i],
                          Lit(s_index(i, j), false)});
    }
  }
  // Force the constraint
  for (int i = 1; i < n; i++) {
    clauses_.push_back({Lit(s_index(i - 1, k - 1), true), ~lits[i]});
  }
}
void SatProblem::None(const std::vector<Lit> &lits) {
  for (Lit l : lits)
    clauses_.push_back({~l});
}
int SatProblem::AddNewVar() {
  ++nr_vars_;
  return nr_vars_ - 1;
}
int SatProblem::AddNewVars(int nr_vars) {
  if (nr_vars <= 0)
    throw "nr_vars should be larger than 0";
  int res = nr_vars_;
  nr_vars_ += nr_vars;
  return res;
}
void SatProblem::AddConstraint(IConstraint *constraint) {
  constraints.push_back(constraint);
}
std::vector<IConstraint*> SatProblem::GetConstraints() const {
  return constraints;
}
std::vector<Lit> SatProblem::GetMinimizeLit() { return minimize_; }
SatProblem::~SatProblem() {
  for (auto c : constraints)
    delete c;
}
SatProblem::SatProblem(const SatProblem &problem) : nr_vars_(problem.nr_vars_){
  clauses_ = problem.clauses_;
  minimize_ = problem.minimize_;
  for (IConstraint * c: problem.constraints) {
    constraints.push_back(c->Clone());
  }

};
} // namespace simple_sat_solver::sat
