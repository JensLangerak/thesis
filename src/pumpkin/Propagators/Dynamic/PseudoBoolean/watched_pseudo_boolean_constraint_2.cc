//
// Created by jens on 24-05-21.
//

#include "watched_pseudo_boolean_constraint_2.h"
#include "../../../Engine/solver_state.h"
#include "../../../Utilities/problem_specification.h"
#include "explanation_pseudo_boolean_constraint_2.h"
#include <algorithm>
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
    lit_weights_[literals[i].ToPositiveInteger()] = weights[i];
    if (weights[i] > max_weight_)
      max_weight_ = weights[i];
    lit_sum_ += weights[i];
  }
  for (BooleanLiteral l : literals) {
    lit_count_[l.ToPositiveInteger()] = 0;
  }
  std::sort(original_literals_.begin(), original_literals_.end(),
            CompareByWeight);
  current_literals_ = std::vector<WeightedLiteral>(original_literals_);
  unencoded_constraint_literals_ =
      std::vector<WeightedLiteral>(original_literals_);
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
  assert(count == current_sum_value_);
  return count == current_sum_value_;
}
void WatchedPseudoBooleanConstraint2::UpdateCounts(
    std::vector<BooleanLiteral> &lits, SolverState &state) {
  propagate_count_++;
  for (BooleanLiteral l : lits) {
    lit_count_[l.ToPositiveInteger()]++;
  }
  if (encoder_->encoding_strategy_ ==
          DYNAMIC ||
      encoder_->encoding_strategy_ ==
          INCREMENTAL) {
    bool add_partial = encoder_->EncodingPartialAdded(); // TODO scheduled

    if (!add_partial) {
      // TODO
      if (encoder_->encoding_strategy_ ==
          INCREMENTAL) {
        int sum = 0;
        for (auto wl : unencoded_constraint_literals_) {
          BooleanLiteral l = wl.literal;
          if (lit_count_[l.ToPositiveInteger()] * wl.weight >
              max_ * encoder_->add_delay) {
            sum += wl.weight;
          }
        }
        add_partial = sum > max_ - max_weight_;
      } else {
        add_partial = propagate_count_ >
                      original_literals_.size() * max_ * encoder_->add_delay;
      }
    }
    if (add_partial) {
      if (!encoder_->EncodingAdded()) {
        state.scheduled_dynamic_constraints_.push_back(this);
        if (encoder_->encoding_strategy_ ==
            INCREMENTAL) {
          if (encoder_->EncodingPartialAdded()) {
            for (BooleanLiteral l : lits) {
              if (lit_count_[l.ToPositiveInteger()] *
                      lit_weights_[l.ToPositiveInteger()] >
                  max_ * encoder_->add_delay)
                add_next_literals_.push_back(l);
            }
          } else {
            for (auto wl : unencoded_constraint_literals_) {
              BooleanLiteral l = wl.literal;
              if (lit_count_[l.ToPositiveInteger()] *
                      lit_weights_[l.ToPositiveInteger()] >
                  max_ * encoder_->add_delay) {
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
    BitStringMap string_map = state.variable_selector_.bit_strings_;
    for (BooleanLiteral l : add_next_literals_) {
      if (encoder_->IsAdded(l))
        continue;
      std::vector<HammingDistanceLiteral> candidates;
      std::vector<int> bit_s = string_map.GetKeyValue(l.VariableIndex() - 1);
      for (auto l2 : unencoded_constraint_literals_) {
        int count_trigger =
            max_ * encoder_->add_delay;
        int l2_count =
            lit_count_[l2.literal.ToPositiveInteger()];
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
      }
      encoder_->Encode(state, add_lits);
    }
    add_next_literals_.clear();

  }

}
} // namespace Pumpkin
