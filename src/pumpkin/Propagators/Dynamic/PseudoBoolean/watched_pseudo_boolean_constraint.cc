//
// Created by jens on 10-12-20.
//

#include "watched_pseudo_boolean_constraint.h"
#include "explanation_pseudo_boolean_constraint.h"
#include "../../../../logger/logger.h"
#include "../../../Engine/solver_state.h"

namespace Pumpkin {

int WatchedPseudoBooleanConstraint2::next_log_id_ = 0;

bool CompareByWeight(const WeightedLiteral &a, WeightedLiteral &b)
{
  return a.weight > b.weight;
}

WatchedPseudoBooleanConstraint2::WatchedPseudoBooleanConstraint2(std::vector<BooleanLiteral> &literals, std::vector<uint32_t> &weights, int max, IEncoder<PseudoBooleanConstraint> *encoder) :
encoder_(encoder), max_(max), current_sum_value(0){
  assert(literals.size() == weights.size());
  max_weight_ = 0;
  for (int i = 0; i < literals.size(); ++i) {
    original_literals_.push_back(WeightedLiteral(literals[i], weights[i]));
    lit_weights_[literals[i].ToPositiveInteger()] = weights[i];
    if (weights[i] > max_weight_)
      max_weight_ = weights[i];
    lit_sum_ += weights[i];
  }
  std::sort(original_literals_.begin(), original_literals_.end(), CompareByWeight);
  current_literals_ = std::vector<WeightedLiteral>(original_literals_);
  unencoded_constraint_literals_ = std::vector<WeightedLiteral>(original_literals_);
  log_id_ = next_log_id_;
  ++next_log_id_;
  encoder_->log_id_ = log_id_;
  simple_sat_solver::logger::Logger::Log2("Constraint: " + std::to_string(log_id_) + " - " + std::to_string(max) + " out of " + std::to_string(literals.size()));
  for (BooleanLiteral l : literals) {
    lit_count_[l.ToPositiveInteger()] = 0;
    lit_decisions_[l.ToPositiveInteger()] = 0;
    lit_prop_[l.ToPositiveInteger()] = 0;
    lit_decisions_[(~l).ToPositiveInteger()] = 0;
    lit_prop_[(~l).ToPositiveInteger()] = 0;
    var_decisions_[l.VariableIndex()] = 0;
  }

  for (int i = 0; i < original_literals_.size(); ++i) {
    for (int j = i + 1; j < original_literals_.size(); ++j) {
      BooleanLiteral l1 = original_literals_[i].literal;
      BooleanLiteral l2 = original_literals_[j].literal;
      if (l1.code_ > l2.code_) {
        std::swap(l1,l2);
      }
      LogLit k1 = LogLit(l1, False);
      LogLit k2 = LogLit(l2, False);

      std::pair<LogLit, LogLit> key(k1,k2);
      lit_logs_counts_conflict[key] = 0;
    }
  }

}

ExplanationPseudoBooleanConstraint2 *
WatchedPseudoBooleanConstraint2::ExplainLiteralPropagation(
    BooleanLiteral literal, SolverState &state) {
  return new ExplanationPseudoBooleanConstraint2(this, state, literal);
}
ExplanationPseudoBooleanConstraint2 *
WatchedPseudoBooleanConstraint2::ExplainFailure(SolverState &state) {
  return new ExplanationPseudoBooleanConstraint2(this, state);
}
WatchedPseudoBooleanConstraint2::~WatchedPseudoBooleanConstraint2() {
  if (encoder_ != nullptr)
    delete encoder_;
}
void WatchedPseudoBooleanConstraint2::UpdateCounts(
    std::vector<BooleanLiteral> &lits, SolverState &state) {
  propagate_count++;
  for (BooleanLiteral l : lits) {
    lit_count_[l.ToPositiveInteger()]++;
//    if (state.assignments_.IsDecision(l.Variable()))
//      lit_decisions_[l.ToPositiveInteger()]++;
  }
  if (!encoder_->AddOnRestart())
    return;
  bool add_partial = encoder_->EncodingPartialAdded(); // TODO scheduled
  if (!add_partial && encoder_->SupportsIncremental()) {
    int sum = 0;
    for (auto wl : unencoded_constraint_literals_) {
      BooleanLiteral l = wl.literal;
      if (lit_count_[l.ToPositiveInteger()] * wl.weight > max_ * encoder_->add_delay) {
        sum+= wl.weight;
      }
    }
    add_partial = sum > max_ - max_weight_;
  } else if (!add_partial && !encoder_->SupportsIncremental()) {
    // TODO
    if (encoder_->add_incremental) {
      add_partial = propagate_count > max_ * encoder_->add_delay;
    } else {
      add_partial = propagate_count > lit_sum_ * max_ * encoder_->add_delay;
    }
  }
  if (add_partial) {
    if (!encoder_->EncodingAdded()) {
      state.propagator_pseudo_boolean_2_.add_constraints_.push(this);
      if (encoder_->EncodingPartialAdded()) {
        for (BooleanLiteral l : lits) {
          if (lit_count_[l.ToPositiveInteger()]*lit_weights_[l.ToPositiveInteger()] > max_ * encoder_->add_delay)
            add_next_literals_.push_back(l);
        }
      } else {
        for (auto wl : unencoded_constraint_literals_) {
          BooleanLiteral l = wl.literal;
          if (lit_count_[l.ToPositiveInteger()]*lit_weights_[l.ToPositiveInteger()] > max_ * encoder_->add_delay) {
            add_next_literals_.push_back(l);
          }
        }
      }
    }
  }
}
void WatchedPseudoBooleanConstraint2::UpdateLog(
    SolverState &state,
    std::unordered_map<std::pair<LogLit, LogLit>, int, LogLitPairHash> &target) {
  return;
  for (int i = 0; i < original_literals_.size(); ++i) {
    for (int j = i + 1; j < original_literals_.size(); ++j) {
      BooleanLiteral l1 = original_literals_[i].literal;
      BooleanLiteral l2 = original_literals_[j].literal;
      if (l1.code_ > l2.code_) {
        std::swap(l1,l2);
      }
      LogLit k1 = LogLit(l1, GetState(state,l1));
      LogLit k2 = LogLit(l2, GetState(state,l2));

      std::pair<LogLit, LogLit> key(k1,k2);
      if (target.count(key) == 0) {
        target[key] = 1;
      } else {
        target[key] += 1;
      }
    }
  }
}
State WatchedPseudoBooleanConstraint2::GetState(SolverState &state,
                                                BooleanLiteral l) {
  if (state.assignments_.IsAssignedTrue(l))
    return True;
  else if (state.assignments_.IsAssignedFalse(l))
    return False;
  else
    return Unassigned;
}
void WatchedPseudoBooleanConstraint2::UpdateConflictCount(SolverState &state) {
  if (!logged_during_conflict)
    UpdateLog(state, lit_logs_counts_conflict);
  logged_during_conflict = true;
}
void WatchedPseudoBooleanConstraint2::UpdatePropagateCount(SolverState &state) {
  return;
  if (!logged_during_conflict)
    UpdateLog(state, lit_logs_counts_conflict);
  logged_during_conflict = true;
}
void WatchedPseudoBooleanConstraint2::UpdateNotTouchedCount(
    SolverState &state) {
  if (!logged_during_conflict)
    UpdateLog(state, lit_logs_counts_not_touched);
}
void WatchedPseudoBooleanConstraint2::UpdateSolutionCount(SolverState &state) {
  return;
  assert(!logged_during_conflict);
  UpdateLog(state, lit_logs_counts_solution);
}
void WatchedPseudoBooleanConstraint2::LogCounts() {
  return;
  simple_sat_solver::logger::Logger::Log2("Count triggers : " + std::to_string(trigger_count_) + " " + std::to_string(conflict_count_));
  for (auto p : lit_logs_counts_solution) {
    if (p.second <= 100)
      continue;
    auto key = p.first;
    simple_sat_solver::logger::Logger::Log2("Count solutions constraint " + std::to_string(log_id_) +" : " +
        std::to_string(key.first.l.code_) + " " + (key.first.state == True ? "T" : key.first.state == False ? "F" : "U") + " - " +
        std::to_string(key.second.l.code_) + " " + (key.second.state == True ? "T" : key.second.state == False ? "F" : "U") + " count " +
        std::to_string(p.second));
  }

  for (auto p : lit_logs_counts_not_touched) {
    auto key = p.first;
    if (key.first.state == Unassigned || key.second.state == Unassigned || (key.first.state == False && key.second.state == False && p.second > 20))
      continue;
    simple_sat_solver::logger::Logger::Log2("Count nottouched constraint " + std::to_string(log_id_) +" : " +
        std::to_string(key.first.l.code_) + " " + (key.first.state == True ? "T" : key.first.state == False ? "F" : "U") + " - " +
        std::to_string(key.second.l.code_) + " " + (key.second.state == True ? "T" : key.second.state == False ? "F" : "U") + " count " +
        std::to_string(p.second));
  }

  for (auto p : lit_logs_counts_propagate) {
    auto key = p.first;
    if (key.first.state == False && key.second.state == False && p.second < 0.3 * this->conflict_count_ || (p.second > 0.1 * this->conflict_count_ && p.second < 0.8 * this->conflict_count_))
      continue;
    simple_sat_solver::logger::Logger::Log2("Count propagate constraint " + std::to_string(log_id_) +" : " +
        std::to_string(key.first.l.code_) + " " + (key.first.state == True ? "T" : key.first.state == False ? "F" : "U") + " - " +
        std::to_string(key.second.l.code_) + " " + (key.second.state == True ? "T" : key.second.state == False ? "F" : "U") + " count " +
        std::to_string(p.second));
  }

  for (auto p : lit_logs_counts_conflict) {
    auto key = p.first;
    if (key.first.state == False && key.second.state == False && p.second < 0.3 * this->conflict_count_ || (p.second > 0.1 * this->conflict_count_ && p.second < 0.8 * this->conflict_count_))
      continue;
    simple_sat_solver::logger::Logger::Log2("Count conflict constraint " + std::to_string(log_id_) +" : " +
        std::to_string(key.first.l.code_) + " " + (key.first.state == True ? "T" : key.first.state == False ? "F" : "U") + " - " +
        std::to_string(key.second.l.code_) + " " + (key.second.state == True ? "T" : key.second.state == False ? "F" : "U") + " count " +
        std::to_string(p.second));
  }
}
bool WatchedPseudoBooleanConstraint2::GetLabel(BooleanLiteral l,
                                               std::string &label) {
  if (encoder_->GetLabel(l, label)) {
    return true;
  } else if (lit_weights_.count(l.ToPositiveInteger())) {
    label = "P_" + std::to_string(log_id_) + "_" + std::to_string(l.code_);
    return true;
  }

  return false;
}
}