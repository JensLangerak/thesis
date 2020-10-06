//
// Created by jens on 05-10-20.
//

#include "cardinality_network.h"
#include <iostream>
#include <ostream>
namespace simple_sat_solver::sat {

std::vector<Lit> CardinalityNetwork::HMerge(std::vector<Lit> a,
                                            std::vector<Lit> b) {
  if (a.size() != b.size())
    throw "A anb b do not have the same size";
  if (a.size() == 0)
    throw "a is empty";
  if (a.size() == 1) {
    std::vector<Lit> c = {Lit(sat_->AddNewVar()), Lit(sat_->AddNewVar())};
    sat_->AddClause({~a[0], ~b[0], c[1]});
    sat_->AddClause({~a[0], c[0]});
    sat_->AddClause({~b[0], c[0]});
    I2O l;
    l.a = a;
    l.b = b;
    l.o = c;
    HMerge_.push_back(l);
    return c;
  } else {
    std::vector<Lit> a_odd;  // i + 1 is odd
    std::vector<Lit> a_even; // i + 1 is even
    std::vector<Lit> b_odd;  // i + 1 is odd
    std::vector<Lit> b_even; // i + 1 is even
    for (int i = 1; i <= a.size(); i++) {
      if (i % 2 == 0) {
        a_even.push_back(a[I(i)]);
        b_even.push_back(b[I(i)]);
      } else {
        a_odd.push_back(a[I(i)]);
        b_odd.push_back(b[I(i)]);
      }
    }
    std::vector<Lit> d = HMerge(a_odd, b_odd);
    std::vector<Lit> e = HMerge(a_even, b_even);

    std::vector<Lit> c;
    c.push_back(d[I(1)]);
    int n = a.size();
    for (int i = 2; i <= n * 2 - 1; ++i) {
      c.emplace_back(sat_->AddNewVar());
    }
    c.push_back(e[I(n)]);
    for (int i = 1; i <= n - 1; ++i) {
      sat_->AddClause({~d[I(i + 1)], ~e[I(i)], c[I(2 * i + 1)]});
      sat_->AddClause({~d[I(i + 1)], c[I(2 * i)]});
      sat_->AddClause({~e[I(i)], c[I(2 * i)]});
    }
    I2O l;
    l.a = a;
    l.b = b;
    l.d = d;
    l.e = e;
    l.o = c;
    HMerge_.push_back(l);
    return c;
  }
}
std::vector<Lit> CardinalityNetwork::HSort(std::vector<Lit> a) {
  if (a.size() == 1) {
    return a;
  } else if (a.size() == 2) {
    std::vector<Lit> res = HMerge({a[0]}, {a[1]});
    I1O l;
    l.a = a;
    l.o = res;
    HSort_.push_back(l);
    return res;
  } else if (a.size() % 2 == 0) {
    int half_size = a.size() / 2;
    std::vector<Lit> l(a.begin(), a.begin() + half_size);
    std::vector<Lit> r(a.begin() + half_size, a.end());
    std::vector<Lit> l_sorted = HSort(l);
    std::vector<Lit> r_sorted = HSort(r);
    std::vector<Lit> res = HMerge(l_sorted, r_sorted);
    I1O l2;
    l2.a = a;
    l2.o = res;
    HSort_.push_back(l2);
    return res;
  } else {
    throw "Not implemented";
  }
}
std::vector<Lit> CardinalityNetwork::SMerge(std::vector<Lit> a,
                                            std::vector<Lit> b) {
  if (a.size() != b.size())
    throw "a anb b not the same size";
  if (a.size() == 0)
    return std::vector<Lit>();
  if (a.size() == 1) {
    std::vector<Lit> c = {Lit(sat_->AddNewVar()), Lit(sat_->AddNewVar())};
    sat_->AddClause({~a[0], ~b[0], c[1]});
    sat_->AddClause({~a[0], c[0]});
    sat_->AddClause({~b[0], c[0]});
    I2O l;
    l.a = a;
    l.b = b;
    l.o = c;
    SMerge_.push_back(l);
    return c;
  } else {
    std::vector<Lit> a_odd;  // i + 1 is odd
    std::vector<Lit> a_even; // i + 1 is even
    std::vector<Lit> b_odd;  // i + 1 is odd
    std::vector<Lit> b_even; // i + 1 is even
    for (int i = 1; i <= a.size(); i++) {
      if (i % 2 == 0) { // TODO figure out why the switch
        a_even.push_back(a[I(i)]);
        b_even.push_back(b[I(i)]);
      } else {
        a_odd.push_back(a[I(i)]);
        b_odd.push_back(b[I(i)]);
      }
    }
    std::vector<Lit> d = SMerge(a_odd, b_odd);
    std::vector<Lit> e = SMerge(a_even, b_even);

    std::vector<Lit> c;
    c.push_back(d[0]);
    int n = a.size();
    for (int i = 2; i <= n + 1; ++i) {
      c.emplace_back(sat_->AddNewVar());
    }
    for (int i = 1; i <= n / 2; ++i) {
      sat_->AddClause({~d[I(i + 1)], ~e[I(i)], c[I(2 * i + 1)]});
      sat_->AddClause({~d[I(i + 1)], c[I(2 * i)]});
      sat_->AddClause({~e[I(i)], c[I(2 * i)]});
    }
    I2O l;
    l.a = a;
    l.b = b;
    l.d = d;
    l.e = e;
    l.o = c;
    SMerge_.push_back(l);
    return c;
  }
}
std::vector<Lit> CardinalityNetwork::Card(std::vector<Lit> a, int k) {
  if (a.size() == k)
    return HSort(a);
  if (a.size() < k)
    throw "a is to small, should be multiple of k";
  std::vector<Lit> l;
  std::vector<Lit> r;
  for (int i = 0; i < k; ++i) {
    l.push_back(a[i]);
  }
  for (int i = k; i < a.size(); i++) {
    r.push_back(a[i]);
  }

  std::vector<Lit> d_l = Card(l, k);
  std::vector<Lit> d_r = Card(r, k);
  std::vector<Lit> c_e = SMerge(d_l, d_r);
  std::vector<Lit> c;
  for (int i = 1; i <= k; ++i) {
    c.push_back(c_e[I(i)]);
  }
  return c;
}
CardinalityNetwork::CardinalityNetwork(SatProblem *sat) : sat_(sat) {}
CardinalityNetwork CardinalityNetwork::Encode(SatProblem &sat,
                                              std::vector<Lit> variables,
                                              int max) {
  if (max < 0)
    throw "Max should be positive";
  if (max > variables.size())
    return CardinalityNetwork(&sat);
  CardinalityNetwork network(&sat);
  int k = 1;
  while (k <= max)
    k *= 2;
  int extra = k - max;
  for (int i = 0; i < extra; ++i) {
    int var = sat.AddNewVar();
    sat.AddClause({~Lit(var)});
    variables.push_back(Lit(var));
  }
  int multiple = variables.size() / k;
  if (variables.size() % k != 0)
    ++multiple;
  extra = multiple * k - variables.size();
  for (int i = 0; i < extra; ++i) {
    int var = sat.AddNewVar();
    sat.AddClause({~Lit(var)});
    variables.push_back(Lit(var));
  }

  std::vector<Lit> c = network.Card(variables, k);
  sat.AddClause({~Lit(c[max])});
  return network;
}
int CardinalityNetwork::I(int i) { return i - 1; }
void CardinalityNetwork::print(std::vector<bool> sol) {
  for (I1O o : HSort_) {
    std::cout << "HSort: " << std::endl;
    print_values(sol, o.a);
    print_values(sol, o.o);
  }

  for (I2O o : HMerge_) {
    std::cout << "HMerge:: " << std::endl;
    print_values(sol, o.a);
    print_values(sol, o.b);
    print_values(sol, o.d);
    print_values(sol, o.e);
    print_values(sol, o.o);
  }

  for (I2O o : SMerge_) {
    std::cout << "SMerge:: " << std::endl;
    print_values(sol, o.a);
    print_values(sol, o.b);
    print_values(sol, o.d);
    print_values(sol, o.e);
    print_values(sol, o.o);
  }
}
void CardinalityNetwork::print_values(std::vector<bool> sol,
                                      std::vector<Lit> lits) {
  for (Lit l : lits) {
    bool s = sol[l.x];
    s = l.complement ? !s : s;
    std::cout << (s ? "1 " : "0 ");
  }
  std::cout << std::endl;
}
} // namespace simple_sat_solver::sat