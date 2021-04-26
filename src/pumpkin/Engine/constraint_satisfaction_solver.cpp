#include "constraint_satisfaction_solver.h"
#include "../Propagators/reason_generic.h"
#include "../Basic Data Structures/runtime_assert.h"

#include <algorithm>
#include <assert.h>
#include <iostream>
#include <random>

#include "../../logger/logger.h"
#include "../Propagators/Dynamic/Encoders/generalized_totaliser.h"
namespace Pumpkin
{

ConstraintSatisfactionSolver::ConstraintSatisfactionSolver(ProblemSpecification& problem_specification, SolverParameters& parameters):
	state_(problem_specification.num_Boolean_variables_, parameters),
	parameters_(parameters),
	counters_(parameters),
	seen_(problem_specification.num_Boolean_variables_ + 1, false),//the 0-th entry is not used
	backtrack_level_(0),
	num_current_decision_level_literals_(0),
	num_trail_literals_examined_(0),
	use_glucose_bumping_(false)
{
  WatchedPseudoBooleanConstraint2::next_log_id_ = 0;
	for (BooleanLiteral unit_literal : problem_specification.unit_clauses_) { state_.AddUnitClause(unit_literal); }
        for (auto& clause : problem_specification.clauses_) { state_.AddClause(clause); }

  simple_sat_solver::logger::Logger::Log2("Before cardinality: v " + std::to_string(state_.GetNumberOfVariables()) + " c_p " + std::to_string(state_.propagator_clausal_.clause_database_.permanent_clauses_.size()) + " c_u " + std::to_string(state_.propagator_clausal_.clause_database_.unit_clauses_.size() ));
  for (auto& constraint : problem_specification.pseudo_boolean_constraints_) { state_.AddPseudoBoolean(constraint); }
  for (auto& constraint : problem_specification.pb_sum_constraints_) { state_.AddPbSum(constraint); }
  for (auto& constraint : problem_specification.cardinality_constraints_) { state_.AddCardinality(constraint); }
  simple_sat_solver::logger::Logger::Log2("After cardinality: v " + std::to_string(state_.GetNumberOfVariables()) + " c_p " + std::to_string(state_.propagator_clausal_.clause_database_.permanent_clauses_.size()) + " c_u " + std::to_string(state_.propagator_clausal_.clause_database_.unit_clauses_.size() ));
  for (auto & constraint : problem_specification.sum_constraints_) { state_.AddSumConstraint(constraint);}
//	if (!problem_specification.pseudo_boolean_constraints_.empty()) { std::cout << "TODO: add pseudo-Boolean constraints!\n"; }
  for (int i =0; i < 21; ++i)
    lbd_histogram[i] = 0;

}


SolverOutput ConstraintSatisfactionSolver::Solve(double time_limit_in_seconds)
{
//  for (auto c : state_.propagator_cardinality_.cardinality_database_.permanent_constraints_) {
//    TotaliserEncoder::Encode(state_, c->literals_, c->min_, c->max_);
//    c->encoding_add = true;
//  }

  Initialise(time_limit_in_seconds);
	//check failure by unit propagation at root
	if (SetUnitClauses() == false) { return SolverOutput(stopwatch_.TimeElapsedInSeconds(), false, std::vector<bool>()); }

  for (auto c : state_.propagator_clausal_.clause_database_.unit_clauses_) {
    assert(state_.assignments_.IsAssignedTrue(c));
  }

  int start_conflict_id = log_conflict_id;
  for (int i = 0; i < 9; ++i) {
    log_conflict_ids[i] = log_conflict_ids[i+1];
  }
  log_conflict_ids[9] = start_conflict_id;

  while (!state_.IsAssignmentBuilt() && stopwatch_.IsWithinTimeLimit())
	{
          state_.IncreaseDecisionLevel();

          BooleanLiteral decision_literal = MakeDecision();
		state_.EnqueueDecisionLiteral(decision_literal);

          PropagatorGeneric* conflicting_propagator = state_.PropagateEnqueued();

		//check if a conflict has been encountered
		if (conflicting_propagator != NULL)
		{
			bool success = ResolveConflict(conflicting_propagator);
			if (success == false) { break; } //UNSAT detected, terminate.
			if (ShouldRestart()) { PerformRestart();}
		}

        }

  simple_sat_solver::logger::Logger::Log2("Total encoded conflicts " + std::to_string(total_encoded_conflicts));
  simple_sat_solver::logger::Logger::Log2("Total encoded conflicts multiple same constraint " + std::to_string(total_encoded_conflicts_more_same_constraint));
  simple_sat_solver::logger::Logger::Log2("Avg lits " + std::to_string(((double)total_lits_in_conflicts) / total_encoded_conflicts));
  simple_sat_solver::logger::Logger::Log2("Avg leaf lits " + std::to_string(((double)total_leaf_lits) / total_encoded_conflicts));
  simple_sat_solver::logger::Logger::Log2("Avg prop lits " + std::to_string(((double)total_propagated_lits) / total_encoded_conflicts));
  simple_sat_solver::logger::Logger::Log2("Avg non const " + std::to_string(((double)total_non_constraint) / total_encoded_conflicts));
  simple_sat_solver::logger::Logger::Log2("Avg encoded lits " + std::to_string(((double )total_encoded_lits) / total_encoded_conflicts));
  simple_sat_solver::logger::Logger::Log2("Avg path " + std::to_string(((double )total_max_path) / total_encoded_conflicts_more_same_constraint));
  simple_sat_solver::logger::Logger::Log2("Avg max same constraint " + std::to_string(((double )total_max_same_constraints) / total_encoded_conflicts));
  simple_sat_solver::logger::Logger::Log2("Avg lbd " + std::to_string(((double )total_lbd_conflicts) / lbd_conflict_count));

  std::string lbd_hist = "Hist lbd ";
  std::string lbd_hist_norm = "Hist norm_lbd ";
  for (int i = 0; i < 21 ; ++i) {
    if (i>0) {
      lbd_hist += " , ";
      lbd_hist_norm += " , ";
    }

    lbd_hist_norm  += std::to_string(((double) lbd_histogram[i]) / lbd_conflict_count);
    lbd_hist  += std::to_string(lbd_histogram[i]);
  }
  simple_sat_solver::logger::Logger::Log2(lbd_hist);
  simple_sat_solver::logger::Logger::Log2(lbd_hist_norm);

//        LogNodeHits(start_conflict_id);


  return GenerateOutput();
}

void ConstraintSatisfactionSolver::PrintStats()
{
	std::cout << "c restarts: " << counters_.restarts;
	if (counters_.restarts == 0)
	{
		std::cout << "\n";
	}
	else
	{
		std::cout << " (" << (counters_.conflicts / counters_.restarts) << " conflicts in avg)" << std::endl;
	}
	std::cout << "c blocked restarts: " << counters_.blocked_restarts << std::endl;
	std::cout << "c nb removed clauses: " << state_.propagator_clausal_.clause_database_.counter_total_removed_clauses_ << std::endl;
	std::cout << "c nb learnts DL2: " << counters_.small_lbd_clauses_learned << std::endl;
	std::cout << "c nb learnts size 1: " << counters_.unit_clauses_learned << std::endl;
	std::cout << "c nb learnts size 3: " << counters_.ternary_clauses_learned << std::endl;
	std::cout << "c nb learnts: " << state_.propagator_clausal_.clause_database_.number_of_learned_clauses_ << std::endl;
	std::cout << "c avg learnt clause size: " << ((long double)state_.propagator_clausal_.clause_database_.number_of_learned_literals_) / state_.propagator_clausal_.clause_database_.number_of_learned_clauses_ << std::endl;
	std::cout << "c current number of learned clauses: " << state_.propagator_clausal_.clause_database_.NumberOfLearnedClauses() << std::endl;
	std::cout << "c ratio of learned clauses: " << double(state_.propagator_clausal_.clause_database_.NumberOfLearnedClauses()) / state_.propagator_clausal_.clause_database_.NumberOfClausesTotal() << std::endl;
	std::cout << "c conflicts: " << counters_.conflicts << std::endl;
	std::cout << "c decisions: " << counters_.decisions << std::endl;
}

void ConstraintSatisfactionSolver::Initialise(double time_limit_in_seconds)
{
	stopwatch_.Initialise(time_limit_in_seconds);
	//since new variables may be introduced by directly accessing the state, the solver might not be notified, and we need to ensure 'seen_' is the right size. This is not an elegant solution and needs to be changed but works for now
	seen_.resize(state_.GetNumberOfVariables() + 1, false);
}

BooleanLiteral ConstraintSatisfactionSolver::MakeDecision()
{
	counters_.decisions++;

	BooleanVariable selected_variable = state_.GetHighestActivityUnassignedVariable();
	state_.variable_selector_.Remove(selected_variable);
	assert(selected_variable.IsUndefined() == false);//this is a "hack" since before calling this function, IsAssignmentBuilt is queries which will prevent this situation from happening
	bool selected_value = state_.value_selector_.SelectValue(selected_variable);
        if (selected_value == false){
          int tes =2;
        }
	BooleanLiteral decision_literal(selected_variable, selected_value);
        for (auto c : state_.propagator_pseudo_boolean_2_.pseudo_boolean_database_.permanent_constraints_) {
          c->lit_decisions_[decision_literal.ToPositiveInteger()]++;
          c->var_decisions_[decision_literal.VariableIndex()]++;
        }
	return decision_literal;
}

bool ConstraintSatisfactionSolver::ResolveConflict(PropagatorGeneric *conflict_propagator)
{
	runtime_assert(conflict_propagator != NULL);

	while (conflict_propagator != NULL)
	{
		UpdateConflictCounters();

		if (state_.GetCurrentDecisionLevel() == 0) { return false; } //conflict found at the root level - unsatisfiability is detected

		ConflictAnalysisResultClausal result = AnalyseConflict(conflict_propagator);
		ProcessConflictAnalysisResult(result);
		conflict_propagator = state_.PropagateEnqueued();
          for (auto c : state_.propagator_pseudo_boolean_2_.pseudo_boolean_database_.permanent_constraints_) {
            c->UpdateNotTouchedCount(state_);
            c->ResetConflictLog();
          }
	}
	state_.propagator_clausal_.clause_database_.DecayClauseActivities();
	state_.variable_selector_.DecayActivities();


	return true; //conflict was successful resolved
}

ConflictAnalysisResultClausal ConstraintSatisfactionSolver::AnalyseConflict(PropagatorGeneric* conflict_propagator)
{
	runtime_assert(conflict_propagator != NULL);
	assert(CheckConflictAnalysisDataStructures());
        if (seen_.size() < state_.GetNumberOfVariables() +1)
          seen_.resize(state_.GetNumberOfVariables() + 1);
        assert(seen_.size() == state_.GetNumberOfVariables() + 1);
	use_glucose_bumping_ = false; //disabling glucose bumping functionality, will enable in the future

	//initialise the analysis with the conflict explanation
	//recall this updates learned_clause_literals_, the num_current_decision_level_literals_, and the backtrack_level_
	ProcessConflictPropagator(conflict_propagator, BooleanLiteral());
	
	//start inspecting the implication graph of conflict analysis
	while (num_current_decision_level_literals_ > 1)
	{
		BooleanLiteral resolution_literal = FindNextReasonLiteralOnTheTrail(); //recall this method will lower the num_current_decision_level_literals counter
		conflict_propagator = state_.assignments_.GetAssignmentPropagator(resolution_literal.Variable());
                auto test = state_.assignments_.info_[resolution_literal.Variable().index_];
		ProcessConflictPropagator(conflict_propagator, resolution_literal);

		seen_[resolution_literal.Variable().index_] = false; //note that we cannot see this variable again in the graph, so we clear its flag here
	}

	//at this point, we know that we expanded all variables of the current decision level that are relevant to the conflict except one literal, which is the first unique implication point
	BooleanLiteral unique_implication_point_literal = FindNextReasonLiteralOnTheTrail();
	state_.variable_selector_.BumpActivity(unique_implication_point_literal.Variable());
	learned_clause_literals_.push_back(~unique_implication_point_literal); //currently the clause generation literal expects the last literal to be the one that is propagating, perhaps I should change this
	assert(CheckDecisionLevelsLearnedLiteral(learned_clause_literals_)); //not sure about this assert, consider removing and replacing elsewhere

	if (use_glucose_bumping_) { PerformGlucoseVariableBumping(); }

	ConflictAnalysisResultClausal result(learned_clause_literals_, ~unique_implication_point_literal, backtrack_level_);
		
	ClearConflictAnalysisDataStructures();
	return result;
}

void ConstraintSatisfactionSolver::ProcessConflictPropagator(PropagatorGeneric *conflict_propagator, BooleanLiteral propagated_literal)
{
  assert(seen_.size() == state_.GetNumberOfVariables() + 1);
  runtime_assert(conflict_propagator != NULL);
	//analyse the reason_constraint of the conflict. 
	//Add new literals to the learned clause_ if their decision level if lower than the current one
	//otherwise increase the number of the number of literals in the current decision level
	ExplanationGeneric *reason_literals;
	
	if (propagated_literal.IsUndefined()) { reason_literals = conflict_propagator->ExplainFailure(state_); }
	else {									reason_literals = conflict_propagator->ExplainLiteralPropagation(propagated_literal, state_); }
	
	for (int i = 0; i < reason_literals->Size(); i++)
	{
		//assigned 
		BooleanLiteral reason_literal = (*reason_literals)[i];
		BooleanVariable reason_variable = reason_literal.Variable();

		//ignore variables at level 0 -> these are unit clauses (in future need to take care, these might be assumptions)
		if (state_.assignments_.GetAssignmentLevel(reason_variable) == 0) { continue; }

		//ignore variable if it was already processed
		if (seen_[reason_variable.index_] == true) { continue; }

		//label it as seen so we do not process the same variable twice in the future for the current conflict
		seen_[reason_variable.index_] = true;

		//experimental for now, might remove
		if (parameters_.bump_decision_variables == true || state_.assignments_.GetAssignmentPropagator(reason_variable) != NULL)
		{
//                  bool is_decision_pb = false;
//                  if (state_.assignments_.IsDecision(reason_variable)) {
//                    for (auto c : state_.propagator_pseudo_boolean_2_.pseudo_boolean_database_.permanent_constraints_) {
//                      for (WeightedLiteral l : c->original_literals_) {
//                        if (l.literal.Variable() == reason_variable) {
//                          is_decision_pb = true;
//                          break;
//                        }
//                      }
//                      if (is_decision_pb)
//                        break;
//                    }
//                  }
//                        if (is_decision_pb || state_.assignments_.GetAssignmentPropagator(reason_variable) == &state_.propagator_pseudo_boolean_2_) {
//                          std::random_device r;
//                          auto gen = std::bind(std::uniform_int_distribution<>(0,1),std::default_random_engine(r()));
//
//                          state_.variable_selector_.BumpActivityExtra(reason_variable,1); //not sure about this one, but I am leaving it for now
////                          bool b = gen();
////                          state_.value_selector_.UpdatePolarity(reason_variable, b);
////                          state_.value_selector_.UpdatePolarity(reason_variable, false, true);//reason_literal.IsNegative());
////                          state_.override_.push(~reason_literal);
//                        } else {
                          state_.variable_selector_.BumpActivity(reason_variable); //not sure about this one, but I am leaving it for now
//                        }
		}

		//either classify the literal as part of a learned clause (if its decision level is smaller than the current level) 
		//or as a candidate for later analysis (if its level is the same as the current level)
		//updates backtrack level and glucose variable candidates appropriately
		int literal_decision_level = state_.assignments_.GetAssignmentLevel(reason_variable);

		if (literal_decision_level == state_.GetCurrentDecisionLevel())
		{
			num_current_decision_level_literals_++;
			if (use_glucose_bumping_) { glucose_bumper_candidates_.push_back(reason_variable); }
		}
		else
		{
			learned_clause_literals_.push_back(~reason_literal);
			backtrack_level_ = std::max(backtrack_level_, literal_decision_level);
		}		
	}
	runtime_assert(num_current_decision_level_literals_ >= 1); //there has to be at least one literal from the current decision level responsible for the failure
	delete reason_literals; //rethink this design, make it wrapped around a smart pointer?
}

void ConstraintSatisfactionSolver::ProcessConflictAnalysisResult(ConflictAnalysisResultClausal& result)
{
  std::string clause="";
  std::vector<std::string> labels;
  bool log = false;
  log_conflict_id++;
          for (auto l : result.learned_clause_literals) {
            clause += " ";

            bool found = false;
            for (auto pb : state_.propagator_pseudo_boolean_2_.pseudo_boolean_database_.permanent_constraints_) {
            std::string label;
            if (pb->GetLabel(l, label)) {
              pb->UpdateNode(l, log_conflict_id);
              clause += label;
              labels.push_back(label);
              log= true;
              found = true;
              break;
              } else {

            }
            }
            if(!found) {
              clause += "N_" + std::to_string(l.code_);
              labels.push_back("N_" + std::to_string(l.code_));
            }
          }
  if (log) {
        total_lits_in_conflicts+=labels.size();
        total_encoded_conflicts += 1;
        int max_path = 0;
        bool encoded_lit = false;
        int max_same_constraint = 0;
        for (int i = 0; i < labels.size();++i) {
            std::string l = labels[i];
            if (l[0] == 'N') {
              total_non_constraint += 1;
              continue;
            }
            if (l[0] == 'P') {
              total_propagated_lits += 1;
              continue;
            }
            if (l[0] == 'E') {
              total_encoded_lits += 1;
              encoded_lit = true;



              int stop = 2;
              while (l[stop] != '_')
                stop += 1;

              int stop_l1 = stop + 1;
              while(l[stop_l1] != '_'  && l[stop_l1] != 'L')
                stop_l1 += 1;

              if (l[stop_l1] == 'L')
                total_leaf_lits += 1;

              int count = 0;
              int stop_l2 = stop + 1;
              for (int j = i + 1; j < labels.size(); ++j) {
                bool same = true;
                std::string l2 = labels[j];
                if (l2[0] != 'E')
                  continue;
                for (int k = 2; k < stop; ++k) {
                  if(l2[k] == '_' || l2[k] == 'L') {
                    stop_l2 = k;
                    break;
                  }

                  if (l[k] != l2[k]) {
                    same = false;
                    break;
                  }
                }
                if (!same)
                  continue;
                count += 1;
                if (max_same_constraint < count)
                  max_same_constraint = count;
                while(l2[stop_l2] != '_' && l2[stop_l2] != 'L')
                  stop_l2 += 1;
                int path_dist = 0;
                int h =stop;
                stop = std::min(stop_l1, stop_l2);
                bool diff = false;
                for (; h < stop; ++h) {
                  if (l[h] != l2[h])
                    diff = true;
                  if (diff)
                    path_dist += 2;
                }
                path_dist += (stop_l1 - h) + (stop_l2 - h);
                if (path_dist > max_path)
                  max_path = path_dist;
              }

            }

          }
          if (max_same_constraint > 0){
            total_max_same_constraints += max_same_constraint;
            total_max_path += max_path;
            total_encoded_conflicts_more_same_constraint +=1;
          }

//          if (encoded_lit)
//          simple_sat_solver::logger::Logger::Log2("Conflict_clause : " + clause);
        }
	//unit clauses are treated in a special way: they are added as decision literals at decision level 0. This might change in the future if a better idea presents itself
	if (result.learned_clause_literals.size() == 1)
	{
		counters_.unit_clauses_learned++;

		state_.Backtrack(0);
		state_.EnqueueDecisionLiteral(result.propagated_literal);
		state_.UpdateMovingAveragesForRestarts(1);
	}
	else
	{
		int lbd = TwoWatchedClause::computeLBD(result.learned_clause_literals, state_) - 1; //minus one since the 1UP will change its decision level
                if (log) {
                  lbd_conflict_count++;
                  total_lbd_conflicts += lbd;
                  if (lbd > 20)
                    lbd_histogram[20] ++;
                  else
                    lbd_histogram[lbd]++;
                }
		state_.UpdateMovingAveragesForRestarts(lbd);

		TwoWatchedClause* learned_clause = state_.AddLearnedClauseToDatabase(result.learned_clause_literals);

		state_.Backtrack(result.backtrack_level);
		state_.EnqueuePropagatedLiteral(result.propagated_literal, &state_.propagator_clausal_, reinterpret_cast<uint64_t>(learned_clause)); //todo can this be done cleaner, without having to refer to how 'code' is interpreted by the propagator?
		
		runtime_assert(lbd == TwoWatchedClause::computeLBD(result.learned_clause_literals, state_));
	}
}

BooleanLiteral ConstraintSatisfactionSolver::FindNextReasonLiteralOnTheTrail()
{
  assert(seen_.size() == state_.GetNumberOfVariables() + 1);
  runtime_assert(num_current_decision_level_literals_ > 0);
	//expand a node of the current decision level
	//find a literal that you have already seen
	//the ones you have not seen are not important for this conflict
	BooleanLiteral next_literal;
        auto it = state_.GetTrailEnd();
	do
	{
          //TODO
          it.Previous();
		next_literal = *it; //state_.GetLiteralFromTheBackOfTheTrail(num_trail_literals_examined_);
		assert(state_.assignments_.GetAssignmentLevel(next_literal.Variable()) == state_.GetCurrentDecisionLevel());
		num_trail_literals_examined_++;
	} while (seen_[next_literal.Variable().index_] == false);

	num_current_decision_level_literals_--; //we are expanding a node and therefore reducing the number of literals of the current level that are left to be considered
	return next_literal;
}

void ConstraintSatisfactionSolver::PerformGlucoseVariableBumping()
{
	runtime_assert(1 == 2); //disabled for now, should enable it! Mainly the problem is when I introduced propagators, conflict clauses are not directly accessible
	/*
	int lbd = TwoWatchedClause::computeLBD(learned_clause_literals_, state_) - 1;
	//	std::cout << glucose_bumper_candidates.Size() << " " << lbd << std::endl;
	for (BooleanVariable var : glucose_bumper_candidates_)
	{
		if (state_.assignments_.GetAssignmentLevel(var) == state_.GetCurrentDecisionLevel())
		{
			if (state_.assignments_.GetAssignmentPropagator(var) != NULL && state_.assignments_.GetAssignmentPropagator(var)->best_literal_blocking_distance_ < lbd)
			{
				state_.variable_selector_.BumpActivity(var);
			}
		}
	}*/
}

void ConstraintSatisfactionSolver::ClearConflictAnalysisDataStructures()
{
	for (BooleanLiteral lit : learned_clause_literals_) { seen_[lit.Variable().index_] = false; } //if I add glucose bumping permanently I can use those literals for clearing rather than clearing in the loop above - might make the code simpler - this is because the literals from the current decision level are already clearned, and only glucose ones are not yet cleared
	learned_clause_literals_.clear();
	glucose_bumper_candidates_.clear();
	backtrack_level_ = 0;
	num_current_decision_level_literals_ = 0; //this is anyway set to zero after conflict analysis
	num_trail_literals_examined_ = 0;
}

bool ConstraintSatisfactionSolver::ShouldRestart()
{ 
	//using the glucose restart strategy as described in "Evaluating CDCL Restart Schemes" -> currently using the original version with simple moving averages instead of the exponential decays

	//if already at the root, do not restart. Note that this affects the cleanup of learned clauses, but for now we ignore this
	if (state_.GetCurrentDecisionLevel() == 0)
	{
		return false;
	}

	//do not consider restarting if the minimum number of conflicts did not occur
	if (counters_.conflicts_until_restart > 0) 
	{
		return false;
	}

	//should postpone the restart? 
	if (counters_.conflicts >= 10000 && state_.GetNumberOfAssignedVariables() > 1.4*state_.simple_moving_average_block.GetCurrentValue())
	{
		counters_.blocked_restarts++;
		counters_.conflicts_until_restart = parameters_.num_min_conflicts_per_restart;
		state_.simple_moving_average_lbd.Reset();
		return false;
	}

	//is the solver learning "bad" clauses?
	if (((long long)state_.simple_moving_average_lbd.GetCurrentValue())*0.8 > state_.cumulative_moving_average_lbd.GetCurrentValue())
	{
		state_.simple_moving_average_lbd.Reset();
		counters_.conflicts_until_restart = parameters_.num_min_conflicts_per_restart;
		return true;
	} else if (!(state_.propagator_pseudo_boolean_2_.add_constraints_.empty() && state_.propagator_pb_sum_input_.add_constraints_.empty())) {
          return true;
        }
	else
	{
		return false;
	}
}

void ConstraintSatisfactionSolver::PerformRestart()
{
//  std::cout << "Restart " << std::endl;
	state_.Backtrack(0);

  if (counters_.until_clause_cleanup <= 0)
	{
		counters_.clause_cleanup++;
		counters_.until_clause_cleanup = parameters_.num_min_conflicts_per_clause_cleanup;

		state_.propagator_clausal_.clause_database_.ReduceTemporaryClauses();
	}

	counters_.restarts++;
	counters_.conflicts_until_restart = parameters_.num_min_conflicts_per_restart;

        state_.AddScheduledEncodings();
        bool res = SetUnitClauses();
        assert(res);
}

void ConstraintSatisfactionSolver::UpdateConflictCounters()
{
	counters_.conflicts++;
	counters_.conflicts_until_restart--;
	counters_.until_clause_cleanup--;
}

bool ConstraintSatisfactionSolver::SetUnitClauses()
{
	for (BooleanLiteral literal : state_.propagator_clausal_.clause_database_.unit_clauses_)
	{
		if (state_.assignments_.IsAssigned(literal))
		{
			assert(state_.assignments_.GetAssignment(literal.Variable()) == literal);
		}
		else
		{
			state_.EnqueueDecisionLiteral(literal);
		}
	}
	PropagatorGeneric* conflict_propagator = state_.PropagateEnqueued();
	return conflict_propagator == NULL;
}

SolverOutput ConstraintSatisfactionSolver::GenerateOutput()
{
	if (!state_.IsAssignmentBuilt())
	{
		return SolverOutput(stopwatch_.TimeElapsedInSeconds(), !stopwatch_.IsWithinTimeLimit(), std::vector<bool>());
	}
	else
	{
          for (auto c : state_.propagator_pseudo_boolean_2_.pseudo_boolean_database_.permanent_constraints_) {
            c->UpdateSolutionCount(state_);
          }
		return SolverOutput(stopwatch_.TimeElapsedInSeconds(), false, state_.GetOutputAssignment());
	}
}

bool ConstraintSatisfactionSolver::CheckConflictAnalysisDataStructures()
{
	assert(IsSeenCleared()); //every element of seen_ is 'false'
	assert(learned_clause_literals_.empty());
	assert(glucose_bumper_candidates_.empty());
	assert(backtrack_level_ == 0);
	assert(num_current_decision_level_literals_ == 0);
	assert(num_trail_literals_examined_ == 0);
	return true;
}

bool ConstraintSatisfactionSolver::IsSeenCleared() const
{
	for (unsigned int i = 0; i < seen_.size(); i++) { assert(seen_[i] == false); }
	return true;
}

bool ConstraintSatisfactionSolver::CheckDecisionLevelsLearnedLiteral(Disjunction & literals)
{
	//assumes the propagating literal is the last one, that might change in the future
	assert(literals.size() > 0);
	assert(state_.assignments_.GetAssignmentLevel(literals.back().Variable()) == state_.GetCurrentDecisionLevel()); //for simplicity I am assuming the current decision level is at the back -> probably will be changed in the future
	for (unsigned int i = 0; i < literals.size() - 1; i++)
	{
		assert(state_.assignments_.GetAssignmentLevel(literals[i].Variable()) < state_.GetCurrentDecisionLevel());
		assert(state_.assignments_.IsAssignedFalse(literals[i]));//this is the same as the next assert, leave only one todo?
		assert(state_.assignments_.GetAssignment(literals[i]) == false);
	}
	return true;
}
void ConstraintSatisfactionSolver::LogNodeHits(int start_conflict_id) {
  for (auto c : state_.propagator_pseudo_boolean_2_.pseudo_boolean_database_.permanent_constraints_) {
    if (c->encoder_->encoding_added_) {
      GeneralizedTotaliser * encoder = (GeneralizedTotaliser *) c->encoder_;
      std::string log_line = "Encoding stats constraint " + std::to_string(c->log_id_) + " ";
      std::vector<int> level_hits;
      std::vector<int> zero_hits;
      std::vector<int> recent_hits;
      std::vector<int> sligly_less_recent_hits;
      UpdateHits(level_hits, zero_hits, recent_hits, sligly_less_recent_hits,0, start_conflict_id, encoder->root_);
      for (int i = 0; i < level_hits.size(); ++i){
        log_line += " " + std::to_string(level_hits[i]) + "-" + std::to_string(zero_hits[i]) + "-" + std::to_string(recent_hits[i]) + "-" + std::to_string(sligly_less_recent_hits[i]);
      }
      simple_sat_solver::logger::Logger::Log2(log_line);
    }

  }
}
void ConstraintSatisfactionSolver::UpdateHits(
    std::vector<int> &level_hits, std::vector<int> &zero_hits,  std::vector<int> &recent_hits, std::vector<int> &slightly_less_recent, int index, int start_conflict_id,
    Pumpkin::GeneralizedTotaliser::Node *node) {
  assert(start_conflict_id >= 0);
  if (node == nullptr)
    return;
  if (level_hits.size() <= index)
    level_hits.push_back(0);
  if (zero_hits.size() <= index)
    zero_hits.push_back(0);
  if (recent_hits.size() <= index)
    recent_hits.push_back(0);
  if (slightly_less_recent.size() <= index)
    slightly_less_recent.push_back(0);

  level_hits[index]+=node->hits;
  if (node->hits == 0)
    zero_hits[index]++;
  if (node->last_conflict_id > start_conflict_id)
    recent_hits[index]++;
  if (node->last_conflict_id > log_conflict_ids[0])
    slightly_less_recent[index]++;

  if (node->left != nullptr)
    UpdateHits(level_hits, zero_hits, recent_hits, slightly_less_recent, index + 1, start_conflict_id, node->left);
  if (node->right != nullptr)
    UpdateHits(level_hits, zero_hits, recent_hits, slightly_less_recent, index + 1,start_conflict_id, node->right);



}

} //end Pumpkin namespace