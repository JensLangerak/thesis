//
// Created by jens on 21-06-21.
//

#include "pb_sum_helper.h"
#include "../../../Engine/solver_state.h"
#include "../Encoders/generalized_totaliser_sum_helpers.h"
#include "Input/propagator_pb_sum_input.h"
#include "Output/propagator_pb_sum_output.h"
#include "watched_pb_sum_constraint.h"
namespace Pumpkin {
PropagatorPbSumInput* PbSumHelper::input_propagator_ = nullptr;
PropagatorPbSumOutput* PbSumHelper::output_propagator_ = nullptr;
WatchedPbSumConstraint *
PbSumHelper::AddConstraint(PbSumConstraint constraint, SolverState &state,
                           GeneralizedTotaliserSumNodes::Factory *encoder_factory) {
  if (input_propagator_ == nullptr)
    AddPropagators(state);
  auto res =  input_propagator_->pb_sum_database_.AddPermanentConstraint(constraint, encoder_factory, state);
  return res;
}
void PbSumHelper::AddPropagators(SolverState &state) {
  assert(input_propagator_ == nullptr);
  assert(output_propagator_ == nullptr);
  assert(state.GetCurrentDecisionLevel() == 0);
  input_propagator_ = new PropagatorPbSumInput(state.GetNumberOfVariables());
  state.AddPropagator(input_propagator_);
  output_propagator_ = new PropagatorPbSumOutput(state.GetNumberOfVariables());
  output_propagator_->pb_sum_database_ = &input_propagator_->pb_sum_database_;
  state.AddPropagator(output_propagator_);
}
}