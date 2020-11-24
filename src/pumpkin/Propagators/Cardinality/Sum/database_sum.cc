//
// Created by jens on 16-10-20.
//

#include "database_sum.h"
namespace Pumpkin {

DatabaseSum::DatabaseSum(uint64_t num_vars)
    : watch_list_true(num_vars) {}

WatchedSumConstraint* DatabaseSum::AddPermanentConstraint(SumConstraint &constraint, SolverState & state) {
    WatchedSumConstraint * watched = new WatchedSumConstraint(constraint.input_literals,constraint.output_literals, constraint.encoder_factory->Create(constraint));
    if (!watched->encoder_->AddEncodingDynamic()) {
      auto res = watched->encoder_->Encode(state);
    }
    permanent_constraints_.push_back(watched);
    AddWatchers(watched);
    return watched;
}
void DatabaseSum::AddWatchers(WatchedSumConstraint *constraint) {
  for (BooleanLiteral lit : constraint->inputs_) {
    watch_list_true.Add(lit, constraint);
  }

}
DatabaseSum::~DatabaseSum() {
  for (WatchedSumConstraint *c : permanent_constraints_) {
    for (BooleanLiteral l : c->inputs_) {
      watch_list_true.Remove(l, c);
    }
//    delete c;
  }

}
} // namespace Pumpkin