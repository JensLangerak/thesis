//
// Created by jens on 15-02-21.
//

#include "static_generalized_totaliser.h"
#include "../../../../logger/logger.h"
#include "../../../Engine/solver_state.h"
#include <iostream>
#include <numeric>
namespace Pumpkin {
std::vector<std::vector<BooleanLiteral>>
Pumpkin::StaticGeneralizedTotaliser::Encode(Pumpkin::SolverState &state) {
  encoding_added_ = true;
  return Encode(state, variables_);
}
std::vector<std::vector<BooleanLiteral>>
StaticGeneralizedTotaliser::Encode(SolverState &state,
                                   std::vector<BooleanLiteral> lits) {
  partial_added_ = true;
  std::vector<std::vector<BooleanLiteral>> added_clauses;
  bool res = false;
  int current_lits = added_lits_.size();
  for (BooleanLiteral l : lits)
    res = AddLiteral(state, l, added_clauses) ? true
                                              : res; // TODO something faster?
  if (!res) { // currently propagation is done in two phases, first the encoding
    // is added, if encoding is not updated, then propagate
    return std::vector<std::vector<BooleanLiteral>>();
  }
  encoding_added_ = added_lits_.size() == variables_.size();
  int new_lits = added_lits_.size();
//  simple_sat_solver::logger::Logger::Log2("Add encoding: ID " + std::to_string(log_id_) + " added vars " + std::to_string(current_lits) + " " + std::to_string(new_lits) + " " +std::to_string(variables_.size()));
  for (auto c : added_clauses) {
    if (c.size() == 1)
      state.AddUnitClause(c[0]);
    else
      state.AddClause(c);
  }

  return added_clauses;
}
bool StaticGeneralizedTotaliser::IsAdded(BooleanLiteral lit) {
  return node_map_[lit.ToPositiveInteger()]->encoded;
}
bool StaticGeneralizedTotaliser::GetLabel(BooleanLiteral l,
                                          std::string &label) {
    if (node_map_.count(l.ToPositiveInteger()) > 0) {
      if (node_map_[l.ToPositiveInteger()]->encoded) {
        label = "E_" + std::to_string(log_id_) + "_" +
                node_map_[l.ToPositiveInteger()]->node_label + "_" +
                std::to_string(l.code_);
        return true;
      }
    }
    return false;
}
void StaticGeneralizedTotaliser::DebugInfo(SolverState &state) {
  return;
}
StaticGeneralizedTotaliser::StaticGeneralizedTotaliser(
    std::vector<BooleanLiteral> variables, std::vector<uint32_t> weights,
    int max) :variables_(variables), weights_(weights), max_(max) {
  assert(max > 1);
  assert(variables_.size() == weights_.size());
  for (int i = 0; i < variables.size(); ++i)
    lit_weights[variables[i].ToPositiveInteger()] = weights[i];

  std::vector<size_t> idx(variables_.size());
  std::iota(idx.begin(), idx.end(), 0);
  std::sort(idx.begin(), idx.end(),
              [&weights](size_t i1, size_t i2) {return weights[i1] > weights[i2];});
  std::vector<BooleanLiteral> leafs;
  leafs.reserve(idx.size());
  for (int i = 0; i < idx.size(); ++i)
    leafs.push_back(variables_[idx[i]]);
  root_= CreateTree(leafs, 0, leafs.size());
  SetLabels(root_, "R" + std::to_string(root_->index));
}
StaticGeneralizedTotaliser::Node *
StaticGeneralizedTotaliser::CreateTree(std::vector<BooleanLiteral> orderedLits,
                                       int startIndex, int endIndex) {
  assert(startIndex < endIndex);
  assert(startIndex >= 0);
  Node * n = new Node();
  if (startIndex + 1 == endIndex) {
    n->nr_leafs = 1;
    BooleanLiteral lit = orderedLits[startIndex];
    n->counting_variables = {lit};
    int w = lit_weights.at(lit.ToPositiveInteger());
    n->counting_variables_weights = {(uint32_t ) w};
    n->index = 1;
    node_map_[lit.ToPositiveInteger()] = n;
    n->max = w;
  } else {
    int split = (startIndex + endIndex) / 2;

    if (split > startIndex) {
      Node *left = CreateTree(orderedLits, startIndex, split);
      n->left = left;
      left->parent = n;
    }
    if (split < endIndex) {
      Node *right = CreateTree(orderedLits, split, endIndex);
      n->right = right;
      right->parent = n;
    }

    n->index = endIndex - startIndex;
    n->nr_leafs = 0;
  }
  return n;
}
bool StaticGeneralizedTotaliser::UpdateMax(int max, SolverState &state) {
  max_ = max;
  if (root_ != nullptr) {
    for (int i = 0; i < root_->counting_variables_weights.size(); ++i) {
      if (root_->counting_variables_weights[i] > max) {
        bool res = state.AddUnitClauseDuringSearch(~root_->counting_variables[i]);
        if (!res)
          return res;
      }
    }
  }

  return true;
}
bool StaticGeneralizedTotaliser::AddLiteral(
    SolverState &state, BooleanLiteral l,
    std::vector<std::vector<BooleanLiteral>> &added_clauses) {
  if (IsAdded(l))
    return false;
  added_lits_[l.ToPositiveInteger()] = true;
  Node * leaf = node_map_[l.ToPositiveInteger()];
  leaf->encoded = true;
  Node * p = leaf->parent;
  while (p != nullptr) {
    p->nr_leafs++;
    if ((p->left == nullptr || p->left->encoded) && (p->right == nullptr || p->right->encoded))
      p->encoded = true;
    p = p->parent;
  }
  UpdateParents(state, leaf, l, added_clauses);
  return true;
}
void StaticGeneralizedTotaliser::UpdateParents(
    SolverState &state, StaticGeneralizedTotaliser::Node *child,
    BooleanLiteral added_lit,
    std::vector<std::vector<BooleanLiteral>> &added_clauses) {
  std::vector<BooleanLiteral> added_lits;
  std::vector<uint32_t> added_weights;
  added_lits.push_back(added_lit);
  added_weights.push_back(child->counting_variables_weights.back());

  while (child->parent != nullptr) {
    Node * parent = child->parent;
    bool is_left_child = parent->left == child;
    std::vector<BooleanLiteral> new_added_lits;
    std::vector<uint32_t > new_added_weights;

    for (int i = 0; i < added_lits.size(); ++i) {
      int q_v = added_weights[i];
      int p_w = q_v;
      int index = GetValueIndex(parent, p_w);
      if (index == -1) {


        index = AddValueIndex(state, parent, p_w);
        new_added_lits.push_back(parent->counting_variables[index]);
        new_added_weights.push_back(parent->counting_variables_weights[index]);

      }
//      parent->clauses.push_back({~added_lits[i], parent->counting_variables[index]});
      added_clauses.push_back({~added_lits[i], parent->counting_variables[index]});
    }

    Node * other = (is_left_child) ? parent->right : parent->left;
    if (other != nullptr) {
      for (int q = 0; q < other->counting_variables_weights.size(); ++q) {
        for (int r = 0; r < added_lits.size(); ++r) {
          int q_v = other->counting_variables_weights[q];
          int r_v = added_weights[r];
          if (q_v > max_|| r_v > max_)
            continue;

          int p_w = q_v + r_v;

          int index = GetValueIndex(parent, p_w);
          if (index == -1) {
            index = AddValueIndex(state, parent, p_w);
            new_added_lits.push_back(parent->counting_variables[index]);
            new_added_weights.push_back(parent->counting_variables_weights[index]);
          }
//          parent->clauses.push_back({~parent->left->counting_variables[q], ~added_lits[r], parent->counting_variables[index]});
          added_clauses.push_back({~other->counting_variables[q], ~added_lits[r], parent->counting_variables[index]});
        }
      }

    }

    added_lits = new_added_lits;
    added_weights = new_added_weights;
    child = child->parent;
  }
}
int StaticGeneralizedTotaliser::GetValueIndex(
    StaticGeneralizedTotaliser::Node *n, int value) {
  assert(value > 0);
  if (value > n->max)
    return -1;
  return n->values_map[value];
}
int StaticGeneralizedTotaliser::AddValueIndex(
    SolverState &state, StaticGeneralizedTotaliser::Node *n, int value) {
  assert(value > 0);
  for (int i = n->max; i <= value; ++i) {
    n->values_map.push_back(-1);
    n->max = i;
  }
  assert(n->values_map[value] == -1);
  BooleanLiteral l = BooleanLiteral(state.CreateNewVariable(), true);
  int index= n->counting_variables.size();
  n->counting_variables.push_back(l);
  node_map_[l.ToPositiveInteger()] = n;
  n->counting_variables_weights.push_back(value);
  n->values_map[value] = index;

  assert(state.GetCurrentDecisionLevel() == 0);
  if (value > max_)
    state.AddUnitClauseDuringSearch(~n->counting_variables[index]);

  return index;
}
StaticGeneralizedTotaliser::~StaticGeneralizedTotaliser() {
  delete root_;
  root_ = nullptr;
}
void StaticGeneralizedTotaliser::SetLabels(
    StaticGeneralizedTotaliser::Node *node, std::string label) {
  if (node == nullptr)
    return;
  if (node->left == nullptr) {
    node->node_label = label + "L";
    return;
  }
  node->node_label = label;
  SetLabels(node->left, label + "l");
  SetLabels(node->right, label + "r");

}
void StaticGeneralizedTotaliser::PrintState(SolverState &state) {

}
StaticGeneralizedTotaliser::Node::~Node() {
  if (left != nullptr)
    delete left;
  if (right != nullptr)
    delete right;

  left = nullptr;
  right = nullptr;
}
}
