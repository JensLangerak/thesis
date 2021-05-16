#include "constraint_optimisation_solver.h"
#include "../Pseudo-Boolean Encoders/EncoderGeneralisedTotaliserCP19_2.h"
#include "../Basic Data Structures/runtime_assert.h"

#include "../../logger/logger.h"
#include "../Propagators/Dynamic/Encoders/propagator_encoder.h"
#include <iostream>
namespace Pumpkin
{

ConstraintOptimisationSolver::ConstraintOptimisationSolver(ProblemSpecification& problem_specification, SolverParameters& parameters) :
	constrained_satisfaction_solver_(problem_specification, parameters),
	lower_bound_(0),
	upper_bound_(INT64_MAX),
	objective_literals_(problem_specification.objective_literals_),
//	encoder_(new GeneralisedTotaliserCP192()),
	activated_(false)
{
//	encoder_->_hax_state = &constrained_satisfaction_solver_.state_;
//	encoder_->objective_literals = objective_literals_;
}

SolverOutput ConstraintOptimisationSolver::Solve(double time_limit_in_seconds)
{
	runtime_assert(activated_ == false); //for now we assume that the solver needs to be called only once. If there is a need for calling multiple times we could look to lift this requirement.
	activated_ = true;

	stopwatch_.Initialise(time_limit_in_seconds);
	constrained_satisfaction_solver_.state_.propagator_clausal_.clause_database_.RecomputeAndPrintClauseLengthStatsForPermanentClauses();

	while (!StoppingCriteriaMet())
	{
		SolverOutput output = constrained_satisfaction_solver_.Solve(stopwatch_.TimeLeftInSeconds());

		if (output.HasSolution())
		{//strenthen the bound of the objective function in the solver
			UpdateBestSolution(output.solution);
                        std::cout << upper_bound_ << std::endl;

			bool success = StrengthenUpperBoundConstraints();
			if (!success) { lower_bound_ = upper_bound_; break; }

			constrained_satisfaction_solver_.state_.value_selector_.PredetermineValues(best_solution_);
//                        for (auto c : constrained_satisfaction_solver_.state_.propagator_pseudo_boolean_2_.pseudo_boolean_database_.permanent_constraints_){
//                          for (auto w : c->unencoded_constraint_literals_) {
//                            constrained_satisfaction_solver_.state_.value_selector_.UpdatePolarity(w.literal.Variable(), w.literal.IsNegative(), true);
//                          }
//                        }

		}
		else if (output.ProvenInfeasible())
		{
                  lower_bound_ = upper_bound_;
                  simple_sat_solver::logger::Logger::Log2("New lowerbound: " + std::to_string(lower_bound_));
		}
	}
        std::cout << upper_bound_ << std::endl;
  for (auto c : constrained_satisfaction_solver_.state_.propagator_pseudo_boolean_2_.pseudo_boolean_database_.permanent_constraints_)
    c->LogCounts();
  for (auto c : constrained_satisfaction_solver_.state_.propagator_pseudo_boolean_3_.pseudo_boolean_database_.permanent_constraints_)
          c->LogCounts();

  int max_lc_ad = 0;
  int max_nc_ad = 0;
  for (auto c : constrained_satisfaction_solver_.state_.propagator_clausal_.clause_database_.temporary_clauses_) {
    if (c->activity_ > max_lc_ad)
      max_lc_ad = c->activity_;
  }
  for (auto n : constrained_satisfaction_solver_.state_.propagator_pseudo_boolean_3_.pseudo_boolean_database_.node_database_.pairs_) {
    for (auto n2 : n.second) {
      for (auto c : n2.second->clauses_) {
        if (c->activity_ > max_nc_ad)
          max_nc_ad = c->activity_;
      }
    }
  }
    std::cout << max_lc_ad << " ---  " << max_nc_ad << std::endl;
	return GenerateOutput();
}

void ConstraintOptimisationSolver::PrintStats()
{
	constrained_satisfaction_solver_.PrintStats();
}

bool ConstraintOptimisationSolver::StrengthenUpperBoundConstraints()
{
	constrained_satisfaction_solver_.state_.Reset(); //for now we restart each time a solution has been found as is usual in MaxSAT. In the future, test NOT restarting but simply continuing by supplying a conflict clause (should be better), possibly setting the restart mechanisms to a fresh start

  if (lower_bound_ == upper_bound_) { return false; }
//	bool success = encoder_->ReduceRightHandSide(upper_bound_ - 1);
        bool success = UpdateBestSolutionConstraint(upper_bound_ - 1);
	//if the encoding added new variables, we set their polarities to zero. Likely this is not an issue since most encodings only add variables the first time the encoding is generated, but this might change in the future
	while (best_solution_.size() <= constrained_satisfaction_solver_.state_.GetNumberOfVariables()) { best_solution_.push_back(false); }

	return success;
}

void ConstraintOptimisationSolver::UpdateBestSolution(const std::vector<bool>& solution)
{
	int64_t new_upper_bound = ComputeSolutionCost(solution);
        simple_sat_solver::logger::Logger::Log2("New solution found: " + std::to_string(new_upper_bound));
        if (new_upper_bound > start_upper_bound_)
          new_upper_bound = start_upper_bound_;


//	runtime_assert(new_upper_bound < upper_bound_);

	upper_bound_ = new_upper_bound;
	best_solution_ = solution;
	solution_time_stamps_.AddTimePoint(stopwatch_.TimeElapsedInSeconds(), upper_bound_);

//	std::cout << "c t=" << stopwatch_.TimeElapsedInSeconds() << "\n";
//	std::cout << "o " << upper_bound_ << "\n";
}

bool ConstraintOptimisationSolver::HasFeasibleSolution() const
{
	return !best_solution_.empty();
}

bool ConstraintOptimisationSolver::StoppingCriteriaMet() const
{
	return lower_bound_ == upper_bound_ || !stopwatch_.IsWithinTimeLimit();
}

SolverOutput ConstraintOptimisationSolver::GenerateOutput() const
{
	if (HasFeasibleSolution())
	{
		return SolverOutput(stopwatch_.TimeElapsedInSeconds(), !stopwatch_.IsWithinTimeLimit(), best_solution_, upper_bound_);
	}
	else
	{
		return SolverOutput(stopwatch_.TimeElapsedInSeconds(), !stopwatch_.IsWithinTimeLimit(), best_solution_, -1);
	}
}

int64_t ConstraintOptimisationSolver::ComputeSolutionCost(const std::vector<bool>& solution) const
{
	int64_t cost = 0;
	for (WeightedLiteral weighted_literal : objective_literals_)
	{
		bool is_violating = IsLiteralTrue(weighted_literal.literal, solution);
		if (is_violating) { cost += weighted_literal.weight; }
	}
	return cost;
}

bool ConstraintOptimisationSolver::IsLiteralTrue(BooleanLiteral literal, const std::vector<bool>& solution)
{
	return literal.IsPositive() && solution[literal.VariableIndex()] || literal.IsNegative() && !solution[literal.VariableIndex()];
}
ConstraintOptimisationSolver::~ConstraintOptimisationSolver() {
//  delete encoder_;
//    if (optimise_constraint != nullptr)
//      delete optimise_constraint;
//    optimise_constraint = nullptr;
}
bool ConstraintOptimisationSolver::UpdateBestSolutionConstraint(int64_t max_cost) {
  if (optimise_constraint == nullptr) {
    std::vector<BooleanLiteral>lits;
    std::vector<uint32_t> coefs;
    auto copy = objective_literals_;
//    std::sort(copy.begin(), copy.end(), [](WeightedLiteral a, WeightedLiteral b){ return a.weight > b.weight; });

    std::string lit_order = "";
    for (auto w : copy) {
      for (auto l : lits) {
        assert (l.Variable() != w.literal.Variable());
      }
      lits.push_back(w.literal);
      coefs.push_back(w.weight);
      lit_order += " " +w.literal.ToString();
    }

    simple_sat_solver::logger::Logger::Log2("Opt lit order: " + lit_order);
    PseudoBooleanConstraint c(lits, coefs, max_cost, optimisation_encoding_factory);
    constrained_satisfaction_solver_.state_.propagator_pseudo_boolean_3_.pseudo_boolean_database_.AddPermanentConstraint(c, constrained_satisfaction_solver_.state_);
    optimise_constraint = constrained_satisfaction_solver_.state_.propagator_pseudo_boolean_3_.pseudo_boolean_database_.permanent_constraints_.back();
//    constrained_satisfaction_solver_.state_.propagator_pseudo_boolean_2_.pseudo_boolean_database_.AddPermanentConstraint(c, constrained_satisfaction_solver_.state_);
//    optimise_constraint = constrained_satisfaction_solver_.state_.propagator_pseudo_boolean_2_.pseudo_boolean_database_.permanent_constraints_.back();
//    optimise_constraint = constrained_satisfaction_solver_.state_.propagator_cardinality_.cardinality_database_.permanent_constraints_.back();

  } else {
    assert(optimise_constraint->max_ >= max_cost);
    optimise_constraint->max_ = max_cost;
    if (!optimise_constraint->encoder_->UpdateMax(max_cost, constrained_satisfaction_solver_.state_))
      return false;
  }

//  if (stopwatch_.TimeElapsedInSeconds() > 300 ) {
//
//    if (optimise_constraint5 == nullptr) {
//      std::vector<BooleanLiteral>lits;
//      std::vector<uint32_t> coefs;
//      auto copy = objective_literals_;
//    std::sort(copy.begin(), copy.end(), [&state = constrained_satisfaction_solver_.state_](WeightedLiteral a, WeightedLiteral b){
//        if (a.weight == b.weight) {
//          double av = state.variable_selector_.heap_.GetKeyValue(a.literal.VariableIndex() - 1);
//          double bv = state.variable_selector_.heap_.GetKeyValue(b.literal.VariableIndex() - 1);
//          return av > bv;
//        }
//        return a.weight > b.weight;
//      });
//
//      std::string lit_order = "";
//      for (auto w : copy) {
//        for (auto l : lits) {
//          assert (l.Variable() != w.literal.Variable());
//        }
//        lits.push_back(w.literal);
//        coefs.push_back(w.weight);
//        lit_order += " " +w.literal.ToString();
//      }
//
//      simple_sat_solver::logger::Logger::Log2("Opt lit order: " + lit_order);
//      PseudoBooleanConstraint c(lits, coefs, max_cost, optimisation_encoding_factory);
//      constrained_satisfaction_solver_.state_.propagator_pseudo_boolean_2_.pseudo_boolean_database_.AddPermanentConstraint(c, constrained_satisfaction_solver_.state_);
//      optimise_constraint5 = constrained_satisfaction_solver_.state_.propagator_pseudo_boolean_2_.pseudo_boolean_database_.permanent_constraints_.back();
////    optimise_constraint5 = constrained_satisfaction_solver_.state_.propagator_cardinality_.cardinality_database_.permanent_constraints_.back();
//
//    } else {
//      assert(optimise_constraint5->max_ >= max_cost);
//      optimise_constraint5->max_ = max_cost;
//      if (!optimise_constraint5->encoder_->UpdateMax(max_cost, constrained_satisfaction_solver_.state_))
//        return false;
//    }
//  }
//
//  if (stopwatch_.TimeElapsedInSeconds() > 600) {
//
//    if (optimise_constraint10 == nullptr) {
//      std::vector<BooleanLiteral>lits;
//      std::vector<uint32_t> coefs;
//      auto copy = objective_literals_;
//      std::sort(copy.begin(), copy.end(), [&state = constrained_satisfaction_solver_.state_](WeightedLiteral a, WeightedLiteral b){
//        if (a.weight == b.weight) {
//          double av = state.variable_selector_.heap_.GetKeyValue(a.literal.VariableIndex() - 1);
//          double bv = state.variable_selector_.heap_.GetKeyValue(b.literal.VariableIndex() - 1);
//          return av > bv;
//        }
//        return a.weight > b.weight;
//      });
//
//      std::string lit_order = "";
//      for (auto w : copy) {
//        for (auto l : lits) {
//          assert (l.Variable() != w.literal.Variable());
//        }
//        lits.push_back(w.literal);
//        coefs.push_back(w.weight);
//        lit_order += " " +w.literal.ToString();
//      }
//      simple_sat_solver::logger::Logger::Log2("Opt lit order: " + lit_order);
//
//
//      PseudoBooleanConstraint c(lits, coefs, max_cost, optimisation_encoding_factory);
//      constrained_satisfaction_solver_.state_.propagator_pseudo_boolean_2_.pseudo_boolean_database_.AddPermanentConstraint(c, constrained_satisfaction_solver_.state_);
//      optimise_constraint10 = constrained_satisfaction_solver_.state_.propagator_pseudo_boolean_2_.pseudo_boolean_database_.permanent_constraints_.back();
////    optimise_constraint10 = constrained_satisfaction_solver_.state_.propagator_cardinality_.cardinality_database_.permanent_constraints_.back();
//
//    } else {
//      assert(optimise_constraint10->max_ >= max_cost);
//      optimise_constraint10->max_ = max_cost;
//      if (!optimise_constraint10->encoder_->UpdateMax(max_cost, constrained_satisfaction_solver_.state_))
//        return false;
//    }
//  }

  return true;
}

}//end Pumpkin namespace