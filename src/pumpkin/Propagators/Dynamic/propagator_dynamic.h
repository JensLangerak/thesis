//
// Created by jens on 28-11-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_PROPAGATOR_DYNAMIC_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_PROPAGATOR_DYNAMIC_H_

#include "../../Basic Data Structures/boolean_literal.h"
#include "../propagator_generic.h"
#include "Encoders/i_encoder.h"
#include <queue>

namespace Pumpkin {
class SolverState;

template <class T, class P>
/// Class that dynamically adds encodings. This class contains the code that is
/// needed to propagate the newly added clauses.
class PropagatorDynamic : public PropagatorGeneric {
public:
  virtual ~PropagatorDynamic() = default;
protected:
  PropagatorDynamic() : PropagatorGeneric() {};
  /// Add the encoding to the clause database.
  /// \param state
  /// \param constraint
  bool AddEncoding(SolverState &state,
                   T *constraint);

  struct PropagtionElement {
    PropagtionElement(BooleanLiteral lit, int level,
                      PropagatorGeneric *propagator, uint64_t code)
        : lit(lit), level(level), propagator(propagator), code(code){};

    BooleanLiteral lit;
    int level;
    uint64_t code;
    PropagatorGeneric *propagator;

    bool operator<(const PropagtionElement o) const {
      return this->level < o.level;
    }

    bool operator>(const PropagtionElement o) const {
      return this->level > o.level;
    }
  };

  /// propagate the literals in the queue to the state.
  int PropagateLiterals(
      std::priority_queue<PropagtionElement, std::vector<PropagtionElement>,
      std::greater<PropagtionElement>>
      &queue,
      SolverState &state, int min_var_index);
  /// propagate the true_literal using the clause database, enques values that can be propagated to the queue. Thus they are not yet added to the state.
  bool ClausualPropagateLiteral(
      BooleanLiteral true_literal, SolverState &state,
      std::priority_queue<PropagtionElement, std::vector<PropagtionElement>,
      std::greater<PropagtionElement>> &queue, int min_var);

  std::vector<std::vector<BooleanLiteral>>

  /// Add some clauses of the encoder.
  AddEncodingClauses(SolverState &state,
                     T *constraint);

  /// Init the propagation queue with newly added clauses that are unit
  std::priority_queue<PropagtionElement, std::vector<PropagtionElement>,
                      std::greater<PropagtionElement>>
  InitPropagationQueue(SolverState &state, int unit_start_index, int clause_start_index);

  /// Based on the newly added clauses, check witch literals should have been propagated before and insert them in the trail.
  /// If the literal is not a newly added variable, backtrack.
  void UpdatePropagation(SolverState &state,
                         std::priority_queue<PropagtionElement, std::vector<PropagtionElement>,
                                             std::greater<PropagtionElement>>&
                         queue, int min_var_index);
  /// Set the position index correct.
  void RepairTrailPositions(SolverState &state);

  /// Get the clauses that trigger the encoding.
  virtual std::vector<BooleanLiteral>
  GetEncodingCause(SolverState &state, T *constraint) = 0;

  /// Propagate incremental. Add the encoding and set some literals.
  bool PropagateIncremental(SolverState &state,
                            T *constraint);
  /// Fill the reason and propagete vectors with the reason for the propagation and the values that should be propagated.
  /// TODO change name.
  virtual void PropagateIncremental2(SolverState &state, T *constraint, std::vector<BooleanLiteral> &reason, std::vector<BooleanLiteral> &propagate) = 0;

  /// Get the encoder for the constraint.
  virtual IEncoder<P> * GetEncoder(T * constraint) = 0;

  /// Do clausual propagation over the added clauses, if propagation is to difficult backtrack.
  void PropagateAddedClauses(SolverState &state, int unit_start_index,
                             int clause_start_index, int var_start_index);
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_PROPAGATOR_DYNAMIC_H_
