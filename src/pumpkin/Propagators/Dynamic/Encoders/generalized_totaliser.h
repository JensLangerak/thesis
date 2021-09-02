//
// Created by jens on 15-12-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_ENCODERS_GENERALIZED_TOTALISER_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_ENCODERS_GENERALIZED_TOTALISER_H_

#include "../../../Utilities/boolean_literal.h"
#include "../../../Utilities/problem_specification.h"
#include "../PseudoBoolean/pseudo_boolean_adder.h"
#include "i_encoder.h"
#include <cassert>
#include <unordered_map>
#include <vector>
namespace Pumpkin {
class GeneralizedTotaliser : public IEncoder<PseudoBooleanConstraint> {

public:
  void Encode(SolverState &state) override;
  void Encode(SolverState &state, std::vector<BooleanLiteral> lits) override;

  bool IsAdded(BooleanLiteral lit) override;

  GeneralizedTotaliser(std::vector<BooleanLiteral> variables, std::vector<uint32_t > weights, int max);
  ~GeneralizedTotaliser() override;
  bool UpdateMax(int max, SolverState &state) override;

  class Factory : public IEncoder<PseudoBooleanConstraint>::IFactory {
  public:
    Factory(EncodingStrategy strategy, double delay_factor) : IEncoder<PseudoBooleanConstraint>::IFactory(strategy, delay_factor) {};
    ~Factory() override = default;

  protected:
    IEncoder<PseudoBooleanConstraint> * CallConstructor(PseudoBooleanConstraint &constraint) override {return new GeneralizedTotaliser(constraint.literals, constraint.coefficients, constraint.right_hand_side);};
  };

//private:
  bool AddLiteral(SolverState &state, BooleanLiteral l, std::vector<std::vector<BooleanLiteral>> &added_clauses);
  std::vector<BooleanLiteral> variables_;
  std::vector<uint32_t > weights_;
  std::unordered_map<int, bool> added_lits_;
  std::unordered_map<int, int> lit_weights_;
  int max_;

  struct Node {
    int index;
    std::vector<BooleanLiteral> counting_variables;
    std::vector<uint32_t> counting_variables_weights;
    std::vector<int> values_map;
    int nr_leafs;

    int max = 0;
    Node *left = nullptr;
    Node *right = nullptr;
    Node *parent = nullptr;
    Node() : left(nullptr), right(nullptr), parent(nullptr), nr_leafs(1) {}
    ~Node();
    std::vector<WeightedLiteral> GetCurrentSumSet();
  };
  std::unordered_map<BooleanLiteral, Node*> node_map;
  Node * root_ = nullptr;
  Node * update_node = nullptr;
  int depth = 0;
  Node *CreateLeaf(SolverState &state, BooleanLiteral literal);
  Node *MergeNode(SolverState & state, Node *node_l, Node *node_r, std::vector<std::vector<BooleanLiteral>> &added_clauses);
  Node *CreateTree(SolverState &state, Node *leaf, int start_index);
  void UpdateNode(SolverState &state, Node *leaf, Node *update_node);
  void UpdateParents(SolverState &state, Node *child, BooleanLiteral added_lit,  std::vector<std::vector<BooleanLiteral>> &added_clauses);
  void SetNextUpdateNode();
  int GetValueIndex(Node * n, int value);
  int AddValueIndex(SolverState &state, Node *n, int value);
  int TreeValid(SolverState &state, Node *node);

  int GetDepth(BooleanLiteral l);
  int Distance(BooleanLiteral l, BooleanLiteral r);
  bool IsEncoded(BooleanLiteral l);
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_ENCODERS_GENERALIZED_TOTALISER_H_
