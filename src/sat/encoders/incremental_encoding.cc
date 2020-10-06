//
// Created by jens on 06-10-20.
//

#include "incremental_encoding.h"
namespace simple_sat_solver::sat {

void IncrementalEncoding::Encode(SatProblem &sat, std::vector<Lit> variables,
                                 std::vector<int> weights, int max) {
  IncrementalEncoding encoder(&sat, variables, weights, max);
  encoder.FindBases();
  encoder.CreateNetwork();
  int b = max;
  for (int i = 0; i < encoder.r_.size(); ++i) {
    int r = encoder.r_[i];
    int j = b %r;
    b = b /r;
    if (j!=0) {
      j = r - j;
      b = b+1;
    }
    if (j == 0) {
      sat.AddClause({~encoder.z_[encoder.ZIndex(1, encoder.w_[i])]});
    } else if(j == r - 1) {
      sat.AddClause({~encoder.z_[encoder.ZIndex(r - 1, encoder.w_[i])]});
    } else {
      sat.AddClause({encoder.z_[encoder.ZIndex(j, encoder.w_[i])]});
      sat.AddClause({~encoder.z_[encoder.ZIndex(j+ 1, encoder.w_[i])]});
    }
  }
  sat.AddClause({~encoder.y_[b - 1]});
}
IncrementalEncoding::IncrementalEncoding(SatProblem *sat,
                                         std::vector<Lit> variables,
                                         std::vector<int> weights, int max)
    : sat_(sat), variables_(variables), weights_(weights), max(max) {}
void IncrementalEncoding::FindBases() {
  // TODO find best base
  r_.push_back(10);
  w_.push_back(1);
  int max = r_[0] * w_[0] - 1;
  int r_index = 0;
  for (int w : weights_) {
    while (w > max) {
      r_.push_back(10);
      w_.push_back(w_[r_index] * r_[r_index]);
      ++r_index;
    }
  }
}
void IncrementalEncoding::CreateNetwork() {
  std::vector<Lit> in;
  std::vector<Lit> carry;
  z_ = std::vector<Lit>();
  std::vector<Lit> out;
  std::vector<int> a = weights_;
  for (int i = 0; i < r_.size(); ++i) {
    in = carry;
    for (int j = 1; j < r_[i]; ++j) {
      Lit var = sat_->AddNewVar();
      z_.push_back(var);
      in.push_back(var);
      if (j >= 2)
        sat_->AddClause({~z_[z_.size() - 1], z_[z_.size() - 2]});
      for (int j = 0; j < variables_.size(); ++j) {
        int v = a[j] % r_[i];
        for (int x = 0; x < v; ++x)
          in.push_back(variables_[v]);
        a[j] = a[j] / r_[i];
      }
      out = EncodeBySorter(in);
      carry.clear();
      // TODO not sure if -1 or -2, i think -1 should be the first carry
      for (j = r_[i] - 1; j < out.size(); j += r_[i])
        carry.push_back(out[j]);
    }
  }
  in = carry;

  for (int i = 0; i < variables_.size(); ++i) {
    for (int j = 0; j < a[i]; ++j) {
      in.push_back(variables_[i]);
    }
  }
  y_ = EncodeBySorter(in);
}
std::vector<Lit> IncrementalEncoding::EncodeBySorter(std::vector<Lit> vars) {
  return std::vector<Lit>();
}
int IncrementalEncoding::ZIndex(int r, int w) {
  int index = r -1;
  for (int i = 0; i < w_.size(); ++i) {
    if (w_[i] < w)
      throw "w not pressent";
    else if (w_[i] == w)
              break;
    else
      index += r_[i];
  }
  return r;

  return  -1;
}
std::vector<Lit> IncrementalEncoding::Oe4Sel(std::vector<Lit> vars, int k) {
  int n = vars.size();
  if (k == 0 || n <= 1)
    return vars;
  else if (k == 1)
    return SelectOne(vars);

  return std::vector<Lit>();
}
std::vector<Lit> IncrementalEncoding::SelectOne(std::vector<Lit> vars) {
  if (vars.size() <= 1)
    return vars;
  else if (vars.size() == 2) {
    Lit c1 = Lit(sat_->AddNewVar());
    Lit c2 = Lit(sat_->AddNewVar());
    sat_->AddClause({c1, ~vars[0]});
    sat_->AddClause({c1, ~vars[1]});
    sat_->AddClause({c2, ~vars[0], ~vars[1]});
    return {c1,c2};
  } else {
    int n = vars.size() / 2;
    std::vector<Lit> l(vars.begin(), vars.begin() + n);
    std::vector<Lit> r(vars.begin() + n, vars.end());
    std::vector<Lit> l_s = SelectOne(l);
    std::vector<Lit> r_s = SelectOne(r);
    std::vector<Lit> c;
    c.push_back(sat_->AddNewVar());
  c.push_back(sat_->AddNewVar());
    sat_->AddClause({c[0], ~l_s[0]});
    sat_->AddClause({c[0], ~r_s[0]});
    sat_->AddClause({c[1], ~l_s[0], ~r_s[0]});
    for (int i = 1; i < l_s.size(); ++i)
      c.push_back(l_s[i]);
    for (int i = 1; i < r_s.size(); ++i)
      c.push_back(r_s[i]);
    return c;
  }
}
}; // namespace simple_sat_solver::sat