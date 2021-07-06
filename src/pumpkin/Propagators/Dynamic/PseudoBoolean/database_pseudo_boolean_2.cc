//
// Created by jens on 24-05-21.
//

#include "database_pseudo_boolean_2.h"
#include "../../../../logger/logger.h"
#include "../../../Engine/solver_state.h"
#include "../../../Utilities/problem_specification.h"
#include "watched_pseudo_boolean_constraint_2.h"
namespace Pumpkin {
DatabasePseudoBoolean2::DatabasePseudoBoolean2(uint64_t num_vars)
    : watch_list_true_(num_vars) {}


WatchedPseudoBooleanConstraint2* DatabasePseudoBoolean2::AddPermanentConstraint(PseudoBooleanConstraint &constraint, IEncoder<PseudoBooleanConstraint>::IFactory *encoding_factory, SolverState & state) {
  WatchedPseudoBooleanConstraint2 * watched = new WatchedPseudoBooleanConstraint2(constraint, encoding_factory);

  std::string line;
  for (BooleanLiteral l : constraint.literals) {
    line += std::to_string(l.VariableIndex()) + " ";
  }
  simple_sat_solver::logger::Logger::Log2("constraint added: id " + std::to_string(watched->log_id_) + " vars " + line);






  if (watched->encoder_->EncodingAddAtStart()) {
    int lits_before = state.GetNumberOfVariables();
    watched->encoder_->Encode(state);
    int lits_after = state.GetNumberOfVariables();
    simple_sat_solver::logger::Logger::Log2("constraint encoded: id " + std::to_string(watched->log_id_) + " vars_first " + std::to_string(lits_before + 1) + " vars_last " + std::to_string(lits_after));
  }
  permanent_constraints_.push_back(watched);
  AddWatchers(watched);
  return watched;
}
void DatabasePseudoBoolean2::AddWatchers(WatchedPseudoBooleanConstraint2 *constraint) {
  for (WeightedLiteral lit : constraint->current_literals_) {
    watch_list_true_.Add(lit.literal, lit.weight, constraint);
  }

}
DatabasePseudoBoolean2::~DatabasePseudoBoolean2() {
  for (WatchedPseudoBooleanConstraint2 *c : permanent_constraints_) {
    for (WeightedLiteral l : c->current_literals_) {
      watch_list_true_.Remove(l.literal, c);
    }
    delete c;
  }

}
}
