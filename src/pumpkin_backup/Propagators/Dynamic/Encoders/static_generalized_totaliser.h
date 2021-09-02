//
// Created by jens on 15-02-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_ENCODERS_STATIC_GENERALIZED_TOTALISER_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_ENCODERS_STATIC_GENERALIZED_TOTALISER_H_

#include "../../../Basic Data Structures/boolean_literal.h"
#include "../../../Basic Data Structures/problem_specification.h"
#include "i_encoder.h"
#include <cassert>
#include <unordered_map>
#include <vector>
namespace Pumpkin {
class StaticGeneralizedTotaliser : public IEncoder<PseudoBooleanConstraint>{

public:
  std::vector<std::vector<BooleanLiteral>>
  Encode(SolverState &state) override;
  std::vector<std::vector<BooleanLiteral>>
  Encode(SolverState &state, std::vector<BooleanLiteral> lits) override;

  std::vector<std::vector<BooleanLiteral>> Propagate(SolverState &state, std::vector<BooleanLiteral> reason, std::vector<BooleanLiteral> propagated_values) override {assert(false);};
  bool IsAdded(BooleanLiteral lit) override;
  bool EncodingAdded() override { return encoding_added_;};
  bool GetLabel(BooleanLiteral l, std::string & label) override;

  void DebugInfo(SolverState &state) override;
  StaticGeneralizedTotaliser(std::vector<BooleanLiteral> variables, std::vector<uint32_t > weights, int max);

  ~StaticGeneralizedTotaliser() override;
  bool SupportsIncremental() override { return add_incremental;} ;
  bool UpdateMax(int max, SolverState &state) override;

  class Factory : public IEncoder<PseudoBooleanConstraint>::IFactory {
    ~Factory() override = default;
    IEncoder<PseudoBooleanConstraint> * CallConstructor(PseudoBooleanConstraint &constraint) override {return new StaticGeneralizedTotaliser(constraint.literals, constraint.coefficients, constraint.right_hand_side);};
  };

//private:
  bool AddLiteral(SolverState &state, BooleanLiteral l, std::vector<std::vector<BooleanLiteral>> &added_clauses);
  std::vector<BooleanLiteral> variables_;
  std::vector<uint32_t > weights_;
  std::unordered_map<int, bool> added_lits_;
  std::unordered_map<int, int> lit_weights;
  std::vector<BooleanLiteral> sum_lits;
  int max_;

  void PrintState(SolverState &state);
  bool AddEncodingDynamic() override { return false;};
  bool AddOnRestart() override { return true;};
  std::vector<WeightedLiteral> GetCurrentSumSet() override {assert(false);};

  struct Node {
    std::string node_label;
    int index;
    std::vector<BooleanLiteral> counting_variables;
    std::vector<uint32_t> counting_variables_weights;
    std::vector<int> values_map;
//    std::vector<uint32_t> variables_weights;
    int nr_leafs;

    bool encoded = false;
    int max = 0;
    Node *left;
    Node *right;
    Node *parent;
    Node() : left(nullptr), right(nullptr), parent(nullptr), nr_leafs(1), index(1) {}
    ~Node();
//    std::vector<WeightedLiteral> GetCurrentSumSet();
  };
  std::unordered_map<int, Node*> node_map_;
  Node * root_ = nullptr;

  Node *CreateTree(std::vector<BooleanLiteral> orderedLits, int startIndex,
                   int endIndex);
  void UpdateParents(SolverState &state, Node *child, BooleanLiteral added_lit,  std::vector<std::vector<BooleanLiteral>> &added_clauses);
  int GetValueIndex(Node * n, int value);
  int AddValueIndex(SolverState &state, Node *n, int value);
  void SetLabels(Node *node, std::string label);
};
}

#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_ENCODERS_STATIC_GENERALIZED_TOTALISER_H_
