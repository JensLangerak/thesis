//
// Created by jens on 10-08-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_EXTENDEDGROUPS_EXTENDED_GROUPS_ADDER_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_EXTENDEDGROUPS_EXTENDED_GROUPS_ADDER_H_
#include "watched_extended_groups_constraint.h"
#include "propagator_extended_groups.h"
namespace Pumpkin {

class PbExtendedGroupsModifier : public IPseudoBooleanConstraintModifier {
public:
  explicit PbExtendedGroupsModifier(WatchedExtendedGroupsConstraint * constraint) : watched_constraint_(constraint) {};
  bool UpdateMax(int max, SolverState & state) override;

protected:
  WatchedExtendedGroupsConstraint * watched_constraint_;
};
class PbExtendedGroupsAdder : public IConstraintAdder<PseudoBooleanConstraint> {
public:
  explicit PbExtendedGroupsAdder(PropagatorExtendedGroups* propagator) : propagator_(propagator) {
  } ;
  void AddConstraint(PseudoBooleanConstraint &c, SolverState &state) override;
  PbExtendedGroupsModifier* AddConstraintWithModifier(PseudoBooleanConstraint &c, SolverState& state) override;
  PropagatorExtendedGroups * GetPropagator() override { return propagator_;}
  ~PbExtendedGroupsAdder() {
  };

protected:
  void AddPropagator(SolverState & state);
  bool propagator_added_= false;
  WatchedExtendedGroupsConstraint * CreateWatchedConstraint(PseudoBooleanConstraint constraint, SolverState & state);
  PropagatorExtendedGroups * propagator_; // TODO

};

}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_DYNAMIC_EXTENDEDGROUPS_EXTENDED_GROUPS_ADDER_H_
