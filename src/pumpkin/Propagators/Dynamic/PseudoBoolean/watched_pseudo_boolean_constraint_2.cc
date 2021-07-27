//
// Created by jens on 24-05-21.
//

#include "watched_pseudo_boolean_constraint_2.h"
#include "../../../Engine/solver_state.h"
#include "../../../Utilities/problem_specification.h"
#include "explanation_pseudo_boolean_constraint_2.h"
#include <algorithm>
#include <unordered_set>

#define HALVE_COUNTS
#define MAX_COUNTS

//#define USE_ACTIVITY

namespace Pumpkin {
bool CompareByWeight(const WeightedLiteral &a, WeightedLiteral &b) {
  return a.weight > b.weight;
}

WatchedPseudoBooleanConstraint2::WatchedPseudoBooleanConstraint2(
    PseudoBooleanConstraint &constraint, IEncoder<PseudoBooleanConstraint>::IFactory * encoding_factory)
    : encoder_(encoding_factory->Create(constraint)),
      max_(constraint.right_hand_side), current_sum_value_(0),
      decision_level_sums_(std::stack<DecisionLevelCount>()) {
  assert(constraint.literals.size() == constraint.coefficients.size());
  auto literals = constraint.literals;
  auto weights = constraint.coefficients;
  max_weight_ = 0;
  for (int i = 0; i < literals.size(); ++i) {
    original_literals_.push_back(WeightedLiteral(literals[i], weights[i]));
    lit_weights_[literals[i]] = weights[i];
    if (weights[i] > max_weight_)
      max_weight_ = weights[i];
    lit_sum_ += weights[i];
  }
  for (BooleanLiteral l : literals) {
    lit_usages_[l.Variable()] = 0;
  }
  std::sort(original_literals_.begin(), original_literals_.end(),
            CompareByWeight);
  current_literals_ = std::vector<WeightedLiteral>(original_literals_);
  for (auto wl : original_literals_)
    unencoded_constraint_literals_.insert(wl.literal);
  log_id_ = next_log_id_;
  ++next_log_id_;
  encoder_->log_id_ = log_id_;
}

ExplanationGeneric *WatchedPseudoBooleanConstraint2::ExplainLiteralPropagation(
    BooleanLiteral literal, SolverState &state,
    ExplanationDynamicConstraint *explanation) {
  ExplanationPseudoBooleanConstraint2::InitExplanationPseudoBooleanConstraint2(
      this, state, literal, explanation);
  return explanation;
}
ExplanationGeneric *WatchedPseudoBooleanConstraint2::ExplainFailure(
    SolverState &state, ExplanationDynamicConstraint *explanation) {
  ExplanationPseudoBooleanConstraint2::InitExplanationPseudoBooleanConstraint2(
      this, state, explanation);
  return explanation;
}
WatchedPseudoBooleanConstraint2::~WatchedPseudoBooleanConstraint2() {
  if (encoder_ != nullptr)
    delete encoder_;
}
void WatchedPseudoBooleanConstraint2::UpdateDecisionLevelSum(
    SolverState &state) {
  if (decision_level_sums_.empty() ||
      decision_level_sums_.top().level < state.GetCurrentDecisionLevel()) {
    decision_level_sums_.push(DecisionLevelCount(
        state.GetCurrentDecisionLevel(), current_sum_value_));
  } else {
    assert(decision_level_sums_.top().level == state.GetCurrentDecisionLevel());
    decision_level_sums_.top().sum_value = current_sum_value_;
  }
}
void WatchedPseudoBooleanConstraint2::Synchronise(SolverState &state) {
  while ((!decision_level_sums_.empty()) &&
         decision_level_sums_.top().level > state.GetCurrentDecisionLevel()) {
    decision_level_sums_.pop();
  }
  if (decision_level_sums_.empty())
    current_sum_value_ = 0;
  else
    current_sum_value_ = decision_level_sums_.top().sum_value;
}
bool WatchedPseudoBooleanConstraint2::CountCorrect(SolverState &state,
                                                   BooleanLiteral literal) {
  if (encoder_->EncodingAdded()) {
    return true;
  }
  std::vector<DecisionLevelCount> stack;
  while (!decision_level_sums_.empty()) {
    DecisionLevelCount countitem = decision_level_sums_.top();
    stack.insert(stack.begin(), countitem);
    decision_level_sums_.pop();
  }
  for (DecisionLevelCount d : stack)
    decision_level_sums_.push(d);

  for (int i = 0; i + 1 < stack.size(); ++i) {
    int level = stack[i].level;
    int sum = stack[i].sum_value;
    int count = 0;

    for (auto wl : original_literals_) {
      if (state.assignments_.IsAssignedTrue(wl.literal) &&
          state.assignments_.GetAssignmentLevel(wl.literal) <= level) {
        count += wl.weight;
        //        std::cout << state.assignments_.GetAssignmentLevel(wl.literal)
        //        << std::endl;
      }
    }
    assert(count == sum);
  }

  int count = 0;
  for (auto wl : original_literals_) {
    if (state.assignments_.IsAssignedTrue(wl.literal) &&
        state.assignments_.GetTrailPosition(wl.literal.Variable()) <=
            state.assignments_.GetTrailPosition(literal.Variable()))
      count += wl.weight;
  }
  int count2 = 0;
  for (auto wl : original_literals_) {
    if (state.assignments_.IsAssignedTrue(wl.literal) &&
        state.assignments_.GetAssignmentLevel(wl.literal.Variable()) <=  state.assignments_.GetAssignmentLevel(literal.Variable()))
      count2 += wl.weight;
  }
  assert(count == current_sum_value_);
  return count == current_sum_value_;
  return true;
}
void WatchedPseudoBooleanConstraint2::UpdateCounts(
    std::vector<BooleanLiteral> &lits, SolverState &state) {
#ifdef HALVE_COUNTS
  if (last_restart_ < state.num_restarts) {
    int factor = pow(2, state.num_restarts - last_restart_);
    propagate_count_ /= factor;
    for (auto wl : unencoded_constraint_literals_)
      lit_usages_[wl.Variable()] /= factor;
  }
#endif
  last_restart_ = state.num_restarts;

  propagate_count_++;
  for (BooleanLiteral l : lits) {
    UpdateLitCount(l);
  }
  if (encoder_->encoding_strategy_ ==
          DYNAMIC ||
      encoder_->encoding_strategy_ ==
          INCREMENTAL) {
    bool add_partial = encoder_->EncodingPartialAdded(); // TODO scheduled

//    int max =std::min(max_, 100);
//    int max = max_;
#ifdef MAX_COUNTS
    int max_i = std::min((int)(max_ * encoder_->add_delay), 1000);
#else
    int max_i = max_ * encoder_->add_delay;
#endif
    if (!add_partial) {
      // TODO
      if (encoder_->encoding_strategy_ ==
          INCREMENTAL) {
        int sum = 0;
        for (auto l : unencoded_constraint_literals_) {
          WeightedLiteral wl = WeightedLiteral(l, lit_weights_[l]);
          if (GetLitCount(l) * wl.weight >
             max_i) {
            sum += wl.weight;
          }
        }
        add_partial = sum > max_ - max_weight_;
      } else {
#ifdef MAX_COUNTS
        int max2 =std::min((int)(max_ *original_literals_.size() * encoder_->add_delay), 1000);
#else
        int max2 = max_ * original_literals_.size() * encoder_->add_delay;
#endif
        add_partial = propagate_count_ > max2;
      }
    }
    if (add_partial) {
      if (!encoder_->EncodingAdded()) {
        state.scheduled_dynamic_constraints_.push_back(this);
        if (encoder_->encoding_strategy_ ==
            INCREMENTAL) {
          if (encoder_->EncodingPartialAdded()) {
            for (BooleanLiteral l : lits) {
              if (encoder_->IsAdded(l))
                continue;
              if (GetLitCount(l) *
                      lit_weights_[l] >
                  max_i)
                add_next_literals_.push_back(l);
            }
          } else {
            for (auto wl : unencoded_constraint_literals_) {
              BooleanLiteral l = wl;
              if (GetLitCount(l)*
                      lit_weights_[l] >
                  max_i) {
                add_next_literals_.push_back(l);
              }
            }
          }
        }
      }
    }
  }
}

struct HammingDistanceLiteral {
  WeightedLiteral l;
  int distance;
  int count;
  HammingDistanceLiteral(WeightedLiteral l, int distance, int count) : l(l), distance(distance), count(count) {};
};

void WatchedPseudoBooleanConstraint2::AddScheduledEncoding(SolverState &state) {
  if (encoder_->EncodingAdded())
    return;

  if (encoder_->encoding_strategy_==DYNAMIC) {
    encoder_->Encode(state);
    add_next_literals_.clear();
  } else if (encoder_->encoding_strategy_ == INCREMENTAL) {
    if (add_next_literals_.empty())
      return;
#ifdef USE_ACTIVITY
    AddUsingActivity(state);
#else
    AddUsingHammingDistance(state);
#endif
    add_next_literals_.clear();

  }

}
void WatchedPseudoBooleanConstraint2::UpdateLitCount(BooleanLiteral lit) {
  BooleanVariable var = lit.Variable();
  if (lit_usages_.count(var) == 0) {
    lit_usages_[var] = 0;
  }

  lit_usages_[var]++;

}
int WatchedPseudoBooleanConstraint2::GetLitCount(BooleanLiteral lit) {
  BooleanVariable var = lit.Variable();
  if (lit_usages_.count(var) == 0)
    return 0;
  return lit_usages_[var];
}
void WatchedPseudoBooleanConstraint2::Reset(SolverState &state) {
  current_sum_value_ = 0;
  while (!decision_level_sums_.empty())
    decision_level_sums_.pop();
}
void WatchedPseudoBooleanConstraint2::AddUsingHammingDistance(
    SolverState &state) {
  BitStringMap string_map = state.variable_selector_.bit_strings_;
  for (BooleanLiteral l : add_next_literals_) {
    if (encoder_->IsAdded(l))
      continue;
    std::vector<HammingDistanceLiteral> candidates;
    std::vector<int> bit_s = string_map.GetKeyValue(l.VariableIndex() - 1);
    for (BooleanLiteral ll2 : unencoded_constraint_literals_) {
      WeightedLiteral l2 = WeightedLiteral(ll2, lit_weights_[ll2]);
      int count_trigger = std::min((int) (
          max_ * encoder_->add_delay), 1000);
      int l2_count =
          GetLitCount(l2.literal);
      int count_factor = l2.weight * l2_count;
      if (count_factor * 1.1 > count_trigger) {
        std::vector<int> bit_s2 =
            string_map.GetKeyValue(l2.literal.VariableIndex() - 1);
        int distance = string_map.HammingDistance(bit_s, bit_s2);
        if (distance > 0.1 * bit_s.size())
          continue;
        HammingDistanceLiteral c =
            HammingDistanceLiteral(l2, distance, l2_count);

        candidates.push_back(c);
      }
    }

    std::sort(candidates.begin(), candidates.end(), [](HammingDistanceLiteral a, HammingDistanceLiteral b) {
      if (a.l.weight != b.l.weight)
        return a.l.weight < b.l.weight;
      return a.distance < b.distance;});

    std::vector<BooleanLiteral> add_lits;
    for (auto c : candidates) {
      add_lits.push_back(c.l.literal);
      unencoded_constraint_literals_.erase(c.l.literal);
    }
    encoder_->Encode(state, add_lits);
  }
}
struct ActiveLit {
  WeightedLiteral l;
  double activity;
  int count;
  ActiveLit(WeightedLiteral l, double act, int count) : l(l), activity(act), count(count) {};
};

void WatchedPseudoBooleanConstraint2::AddUsingActivity(SolverState &state) {
  std::vector<ActiveLit> lits;
  std::unordered_set<BooleanLiteral> added_lits;
  for (auto lit :add_next_literals_) {
    if (added_lits.count(lit) == 0) {
      added_lits.insert(lit);
      int w = lit_weights_[lit];
      WeightedLiteral wl = WeightedLiteral(lit, w);
      double acitivity = state.variable_selector_.GetActiviy(lit.Variable());
      int count = lit_usages_[lit.Variable()];
      lits.push_back(ActiveLit(wl, acitivity, count));
    }

  }
  std::sort(lits.begin(), lits.end(), [](ActiveLit a, ActiveLit b) {
    if (a.l.weight != b.l.weight)
      return a.l.weight < b.l.weight;
    return a.activity > b.activity;});
  std::vector<BooleanLiteral> add_lits;
  for (auto c : lits) {
    add_lits.push_back(c.l.literal);
    unencoded_constraint_literals_.erase(c.l.literal);
  }
  encoder_->Encode(state, add_lits);

}
} // namespace Pumpkin
