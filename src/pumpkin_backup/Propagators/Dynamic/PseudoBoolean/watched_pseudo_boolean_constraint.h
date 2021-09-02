//
// Created by jens on 10-12-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_WATCHED_PSEUDO_BOOLEAN_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_WATCHED_PSEUDO_BOOLEAN_CONSTRAINT_H_

#include "../../../Basic Data Structures/boolean_literal.h"
#include "../Encoders/i_encoder.h"
#include "../../../Basic Data Structures/problem_specification.h"
#include <unordered_map>
namespace Pumpkin {
class ExplanationPseudoBooleanConstraint2;
class SolverState;
enum State {True, False, Unassigned};
struct LogLit {
    BooleanLiteral l;
    State state;
    LogLit(BooleanLiteral l, State s) : l(l), state(s) {};

  bool operator<(const LogLit& r) const {
    return (l.code_<r.l.code_ || (l.code_==r.l.code_ && state<r.state));
  }

  bool operator==(const LogLit &o) const {
    return l.code_ == o.l.code_ && state == o.state;
  }
};


struct LogLitPairHash {
  size_t operator()(std::pair<LogLit, LogLit> p) const noexcept {
    size_t h1 = p.first.l.code_ << 3 ^ p.first.state;
    size_t h2 = p.second.l.code_ << 3 ^ p.second.state;
    return h1 << 16 ^ h2;
  }
};

class WatchedPseudoBooleanConstraint2 {
public:
  WatchedPseudoBooleanConstraint2(std::vector<BooleanLiteral> &literals, std::vector<uint32_t> &weights, int max, IEncoder<PseudoBooleanConstraint> *encoder);
  ~WatchedPseudoBooleanConstraint2();
  ExplanationPseudoBooleanConstraint2 *
  ExplainLiteralPropagation(BooleanLiteral literal, SolverState &state); // returns the conjunction that forces the assignment of input
  // literal to true. Assumes the input literal is not undefined.
  ExplanationPseudoBooleanConstraint2 *ExplainFailure(SolverState &state); // returns the conjunction that leads to failure

//  struct WeightedLiteral {
//    WeightedLiteral(BooleanLiteral l, int w) : lit(l), weight(w) {};
//    BooleanLiteral lit;
//    int weight;
//  };

  std::vector<WeightedLiteral> current_literals_;
  std::vector<WeightedLiteral> original_literals_;
  std::vector<WeightedLiteral> unencoded_constraint_literals_;
//  std::vector<WeightedLiteral> added_to_encoding_literals_;
//  std::vector<WeightedLiteral> encoded_sum_literals_;
  int max_;

  int current_sum_value;
  int trigger_count_ = 0;
  int conflict_count_ = 0;
  IEncoder<PseudoBooleanConstraint> *encoder_;
  int log_id_;
  static int next_log_id_;
  uint32_t max_weight_;
  std::unordered_map<int, int> lit_count_;
  std::unordered_map<int, int> lit_decisions_;
  std::unordered_map<int, int> var_decisions_;
  std::unordered_map<int, int> lit_prop_;
  std::unordered_map<int, uint32_t> lit_weights_;
  int propagate_count =0;
  std::vector<BooleanLiteral> add_next_literals_;
  int true_count_debug_ = 0;
  uint32_t lit_sum_ =0 ;
  void UpdateCounts(std::vector<BooleanLiteral> &lits, SolverState &state);
  bool logged_during_conflict = false;
  std::unordered_map<std::pair<LogLit, LogLit>,int, LogLitPairHash> lit_logs_counts_conflict;
  std::unordered_map<std::pair<LogLit, LogLit>,int, LogLitPairHash> lit_logs_counts_propagate;
  std::unordered_map<std::pair<LogLit, LogLit>,int, LogLitPairHash> lit_logs_counts_solution;
  std::unordered_map<std::pair<LogLit, LogLit>,int, LogLitPairHash> lit_logs_counts_not_touched;
  State GetState(SolverState &state, BooleanLiteral l);
  void UpdateLog(SolverState& state, std::unordered_map<std::pair<LogLit, LogLit>, int, LogLitPairHash> &target);
  void UpdateConflictCount(SolverState & state);
  void UpdatePropagateCount(SolverState & state);
  void UpdateNotTouchedCount(SolverState & state);
  void UpdateSolutionCount(SolverState & state);
  void ResetConflictLog() {
    if (logged_during_conflict)
      conflict_count_++;
    logged_during_conflict = false;
  }
  void LogCounts();

  bool GetLabel(BooleanLiteral l, std::string & label);

  void UpdateNode(BooleanLiteral literal, int conflict_id);
};
} // namespace Pumpkin
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_PSEUDOBOOLEAN_WATCHED_PSEUDO_BOOLEAN_CONSTRAINT_H_
