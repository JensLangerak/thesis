//
// Created by jens on 15-12-20.
//

#include "generalized_totaliser.h"
#include "../../../Engine/solver_state.h"
#include <iostream>
#include <numeric>
namespace Pumpkin {
void GeneralizedTotaliser::Encode(SolverState &state) {
  encoding_added_ = true;
  Encode(state, variables_);
}

void GeneralizedTotaliser::Encode(SolverState &state,
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
    assert(res);
    return;
  }
  encoding_added_ = added_lits_.size() == variables_.size();
  //  simple_sat_solver::logger::Logger::Log2("Add encoding: ID " +
  //  std::to_string(log_id_) + " added vars " + std::to_string(current_lits) +
  //  " " + std::to_string(new_lits) + " " +std::to_string(variables_.size()));
  for (auto c : added_clauses) {
    if (c.size() == 1)
      state.AddUnitClause(c[0]);
    else
      state.AddClause(c);
  }
}

bool GeneralizedTotaliser::IsAdded(BooleanLiteral lit) {
  auto it = added_lits_.find(lit.ToPositiveInteger());
  if (it == added_lits_.end()) {
    return false;
  }
  return it->second;
}
GeneralizedTotaliser::GeneralizedTotaliser(
    std::vector<BooleanLiteral> variables, std::vector<uint32_t> weights,
    int max)
    : variables_(variables), weights_(weights), max_(max) {
  assert(max > 1);
  assert(variables_.size() == weights_.size());
  for (int i = 0; i < variables.size(); ++i)
    lit_weights_[variables[i].ToPositiveInteger()] = weights[i];
}
bool GeneralizedTotaliser::UpdateMax(int max, SolverState &state) {
  max_ = max;
  if (root_ != nullptr) {
    for (int i = 0; i < root_->counting_variables_weights.size(); ++i) {
      if (root_->counting_variables_weights[i] > max) {
        bool res = state.AddUnitClause(~root_->counting_variables[i]);
        if (!res)
          return res;
      }
    }
  }
  return true;
}
bool GeneralizedTotaliser::AddLiteral(
    SolverState &state, BooleanLiteral l,
    std::vector<std::vector<BooleanLiteral>> &added_clauses) {
  if (IsAdded(l)) // TODO assert?
    return false;

  added_lits_[l.ToPositiveInteger()] = true;

  Node *leaf = CreateLeaf(state, l);
  if (root_ == nullptr) {
    root_ = leaf;
    update_node = root_;
    depth = 1;
    return true;
  }

  assert(update_node != nullptr);
  if (update_node->index == update_node->nr_leafs) {
    assert(update_node->parent == nullptr);
    assert(root_ == update_node);
    Node *second_tree = CreateTree(state, leaf, update_node->index);
    UpdateParents(state, leaf, l, added_clauses);
    root_ = MergeNode(state, update_node, second_tree, added_clauses);
    update_node = leaf->parent;
  } else {
    assert(update_node->index > update_node->nr_leafs);
    assert(update_node->left != nullptr &&
           update_node->left->index == update_node->left->nr_leafs);
    UpdateNode(state, leaf, update_node);
    UpdateParents(state, leaf, l, added_clauses);
    update_node = leaf;
    SetNextUpdateNode();
  }
  return true;
}
GeneralizedTotaliser::Node *
GeneralizedTotaliser::CreateLeaf(SolverState &state, BooleanLiteral literal) {
  Node *n = new Node();
  //  n->variables.push_back(literal);
  n->counting_variables.push_back(literal);
  node_map[literal.ToPositiveInteger()] = n;
  uint32_t w = lit_weights_[literal.ToPositiveInteger()];
  n->counting_variables_weights.push_back(w);
  //  n->variables_weights.push_back(w);
  n->index = 1;
  n->max = n->counting_variables_weights[0];
  return n;
}
GeneralizedTotaliser::Node *GeneralizedTotaliser::MergeNode(
    SolverState &state, GeneralizedTotaliser::Node *node_l,
    GeneralizedTotaliser::Node *node_r,
    std::vector<std::vector<BooleanLiteral>> &added_clauses) {
  Node *p = new Node();
  p->index = node_l->index + node_r->index;
  p->nr_leafs = node_l->nr_leafs + node_r->nr_leafs;
  p->left = node_l;
  p->right = node_r;
  node_l->parent = p;
  node_r->parent = p;

  for (int i = 0; i < node_l->counting_variables.size(); i++) {
    int q_v = node_l->counting_variables_weights[i];
    int p_w = q_v;
    int index = GetValueIndex(p, p_w);
    if (index == -1) {
      index = AddValueIndex(state, p, p_w);
    }
    //    p->clauses.push_back({~node_l->counting_variables[i],
    //    p->counting_variables[index]});
    added_clauses.push_back(
        {~node_l->counting_variables[i], p->counting_variables[index]});
  }

  for (int i = 0; i < node_r->counting_variables.size(); i++) {
    int r_v = node_r->counting_variables_weights[i];
    int p_w = r_v;
    int index = GetValueIndex(p, p_w);
    if (index == -1) {
      index = AddValueIndex(state, p, p_w);
    }
    //    p->clauses.push_back({~node_r->counting_variables[i],
    //    p->counting_variables[index]});
    added_clauses.push_back(
        {~node_r->counting_variables[i], p->counting_variables[index]});
  }

  for (int q = 0; q < node_l->counting_variables.size(); ++q) {
    for (int r = 0; r < node_r->counting_variables.size(); ++r) {
      int q_v = node_l->counting_variables_weights[q];
      int r_v = node_r->counting_variables_weights[r];
      if (q_v > max_ || r_v > max_)
        continue;
      int p_w = q_v + r_v;
      int index = GetValueIndex(p, p_w);
      if (index == -1) {
        index = AddValueIndex(state, p, p_w);
      }

      //      p->clauses.push_back({~node_l->counting_variables[q],
      //      ~node_r->counting_variables[r], p->counting_variables[index]});
      added_clauses.push_back({~node_l->counting_variables[q],
                               ~node_r->counting_variables[r],
                               p->counting_variables[index]});
    }
  }
  for (int i = 0; i < p->counting_variables_weights.size(); ++i) {
    if (p->counting_variables_weights[i] > max_)
      added_clauses.push_back({~p->counting_variables[i]}); // TODO
  }
  //  std::vector<size_t> idx(p->counting_variables_weights.size());
  //  std::iota(idx.begin(), idx.end(), 0);
  //  std::sort(idx.begin(), idx.end(), [&p](size_t i1, size_t i2) { return
  //  p->counting_variables_weights[i1] < p->counting_variables_weights[i2];});
  //  for (int i =1; i < idx.size(); ++i) {
  //    added_clauses.push_back({p->counting_variables[i-1],
  //    ~p->counting_variables[i]});
  //  }

  return p;
}
GeneralizedTotaliser::Node *GeneralizedTotaliser::CreateTree(
    SolverState &state, GeneralizedTotaliser::Node *leaf, int start_index) {
  assert(start_index > 0);
  assert(start_index % 2 == 0 || start_index == 1);
  if (start_index == 1)
    return leaf;
  Node *n = new Node();
  n->index = start_index;
  n->left = CreateTree(state, leaf, start_index / 2);
  n->left->parent = n;
  return n;
}
void GeneralizedTotaliser::UpdateNode(SolverState &state,
                                      GeneralizedTotaliser::Node *leaf,
                                      GeneralizedTotaliser::Node *update_node) {
  assert(update_node->index != update_node->nr_leafs);
  assert(update_node->left->index == update_node->left->nr_leafs);
  assert(update_node->right == nullptr);
  update_node->right = CreateTree(state, leaf, update_node->left->index);
  update_node->right->parent = update_node;
  Node *p = update_node;
  while (p != nullptr) {
    p->nr_leafs++;
    p = p->parent;
  }
}
void GeneralizedTotaliser::UpdateParents(
    SolverState &state, GeneralizedTotaliser::Node *child,
    BooleanLiteral added_lit,
    std::vector<std::vector<BooleanLiteral>> &added_clauses) {
  std::vector<BooleanLiteral> added_lits;
  std::vector<uint32_t> added_weights;
  added_lits.push_back(added_lit);
  added_weights.push_back(child->counting_variables_weights.back());

  while (child->parent != nullptr) {
    Node *parent = child->parent;
    std::vector<BooleanLiteral> new_added_lits;
    std::vector<uint32_t> new_added_weights;

    for (int i = 0; i < added_lits.size(); ++i) {
      int q_v = added_weights[i];
      int p_w = q_v;
      int index = GetValueIndex(parent, p_w);
      if (index == -1) {
        int lower = -1;
        int higher = -1;
        for (int j = 0; j < parent->counting_variables_weights.size(); j++) {
          int w = parent->counting_variables_weights[j];
          if (w < p_w) {
            if (lower == -1 || w > parent->counting_variables_weights[lower])
              lower = j;
          }
          if (w > p_w) {
            if (higher == -1 || w < parent->counting_variables_weights[higher])
              higher = j;
          }
        }

        index = AddValueIndex(state, parent, p_w);
        new_added_lits.push_back(parent->counting_variables[index]);
        new_added_weights.push_back(parent->counting_variables_weights[index]);

        //        if (lower > -1)
        //          added_clauses.push_back({parent->counting_variables[lower],
        //          ~parent->counting_variables[index]});
        //        if (higher > -1)
        //          added_clauses.push_back({~parent->counting_variables[higher],
        //          parent->counting_variables[index]});
      }
      //      parent->clauses.push_back({~added_lits[i],
      //      parent->counting_variables[index]});
      added_clauses.push_back(
          {~added_lits[i], parent->counting_variables[index]});
    }

    if (child == parent->right) {
      for (int q = 0; q < parent->left->counting_variables.size(); ++q) {
        for (int r = 0; r < added_lits.size(); ++r) {
          int q_v = parent->left->counting_variables_weights[q];
          int r_v = added_weights[r];
          if (q_v > max_ || r_v > max_)
            continue;

          int p_w = q_v + r_v;

          int index = GetValueIndex(parent, p_w);
          if (index == -1) {
            int lower = -1;
            int higher = -1;
            for (int j = 0; j < parent->counting_variables_weights.size();
                 j++) {
              int w = parent->counting_variables_weights[j];
              if (w < p_w) {
                if (lower == -1 ||
                    w > parent->counting_variables_weights[lower])
                  lower = j;
              }
              if (w > p_w) {
                if (higher == -1 ||
                    w < parent->counting_variables_weights[higher])
                  higher = j;
              }
            }
            index = AddValueIndex(state, parent, p_w);
            new_added_lits.push_back(parent->counting_variables[index]);
            new_added_weights.push_back(
                parent->counting_variables_weights[index]);
            //            if (lower > -1)
            //              added_clauses.push_back({parent->counting_variables[lower],
            //              ~parent->counting_variables[index]});
            //            if (higher > -1)
            //              added_clauses.push_back({~parent->counting_variables[higher],
            //              parent->counting_variables[index]});
          }
          //          parent->clauses.push_back({~parent->left->counting_variables[q],
          //          ~added_lits[r], parent->counting_variables[index]});
          added_clauses.push_back({~parent->left->counting_variables[q],
                                   ~added_lits[r],
                                   parent->counting_variables[index]});
        }
      }
    }

    added_lits = new_added_lits;
    added_weights = new_added_weights;
    child = child->parent;
  }
}
void GeneralizedTotaliser::SetNextUpdateNode() {
  assert(update_node->index == update_node->nr_leafs);
  while (update_node->index == update_node->nr_leafs && update_node != root_)
    update_node = update_node->parent;
  if (update_node == root_)
    return;
  assert(update_node->right == nullptr);
  //  if (update_node->right != nullptr)
  //    update_node = update_node->right;
}
GeneralizedTotaliser::~GeneralizedTotaliser() {
  if (root_ != nullptr)
    delete root_;
}
int GeneralizedTotaliser::GetValueIndex(GeneralizedTotaliser::Node *n,
                                        int value) {
  assert(value > 0);
  if (value > n->max)
    return -1;
  return n->values_map[value];
}
int GeneralizedTotaliser::AddValueIndex(SolverState &state,
                                        GeneralizedTotaliser::Node *n,
                                        int value) {
  assert(value > 0);
  for (int i = n->max; i <= value; ++i) {
    n->values_map.push_back(-1);
    n->max = i;
  }
  assert(n->values_map[value] == -1);
  BooleanLiteral l = BooleanLiteral(state.CreateNewVariable(), true);
  int index = n->counting_variables.size();
  n->counting_variables.push_back(l);
  node_map[l.ToPositiveInteger()] = n;
  n->counting_variables_weights.push_back(value);
  n->values_map[value] = index;

  assert(state.GetCurrentDecisionLevel() == 0);
  if (value > max_)
    state.AddUnitClause(~n->counting_variables[index]);

  return index;
}
int GeneralizedTotaliser::TreeValid(SolverState &state,
                                    GeneralizedTotaliser::Node *node) {
  uint32_t value = 0;
  for (int i = 0; i < node->counting_variables.size(); i++) {
    if (state.assignments_.IsAssignedTrue(node->counting_variables[i]))
      value = std::max(value, node->counting_variables_weights[i]);
  }
  for (int i = 0; i < node->counting_variables.size(); i++) {
    assert(node->counting_variables_weights[i] <= value ==
           state.assignments_.IsAssignedTrue(node->counting_variables[i]));
  }

  int sum = 0;
  int s_l = 0;
  int s_r = 0;
  if (node->left != nullptr)
    s_l = TreeValid(state, node->left);

  if (node->right != nullptr)
    s_r = TreeValid(state, node->right);
  sum = s_l + s_r;

  if (node->left != nullptr || node->right != nullptr) {
    if (sum > value) {
      //      for (auto c : node->clauses) {
      //        bool v = false;
      //        for (BooleanLiteral l : c)
      //          v |= state.assignments_.IsAssignedTrue(l);
      //        assert(v);
      //      }

      assert(sum <= value);
    }
  }

  return value;
}

GeneralizedTotaliser::Node::~Node() {
  delete left;
  delete right;
}
std::vector<WeightedLiteral> GeneralizedTotaliser::Node::GetCurrentSumSet() {
  // balanced tree
  if (index == 1) { // nr_leafs) {
    // TODO store result
    std::vector<WeightedLiteral> result;
    for (int i = 0; i < counting_variables.size(); ++i) {
      result.push_back(WeightedLiteral(counting_variables[i],
                                       counting_variables_weights[i]));
    }
    return result;
  }
  std::vector<WeightedLiteral> left_w = left->GetCurrentSumSet();
  std::vector<WeightedLiteral> right_w;
  if (right != nullptr)
    right_w = right->GetCurrentSumSet();
  left_w.reserve(left_w.size() + right_w.size());
  left_w.insert(left_w.end(), right_w.begin(), right_w.end());
  return left_w;
}
} // namespace Pumpkin
