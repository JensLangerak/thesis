//
// Created by jens on 28-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_INCREMENTAL_SEQUENTIAL_ENCODER_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_INCREMENTAL_SEQUENTIAL_ENCODER_H_

#include "../../../Basic Data Structures/boolean_literal.h"
#include "i_encoder.h"
#include <vector>
namespace Pumpkin {
class IncrementalSequentialEncoder : public IEncoder {

public:
  std::vector<std::vector<BooleanLiteral>>
  Encode(SolverState &state) override;
  std::vector<std::vector<BooleanLiteral>>
  Encode(SolverState &state, std::vector<BooleanLiteral> lits) override;
  bool IsAdded(BooleanLiteral lit);


  IncrementalSequentialEncoder(std::vector<BooleanLiteral> variables, int min, int max);

  ~IncrementalSequentialEncoder() override;
  bool SupportsIncremental() override { return true;} ;


private:
  bool AddLiteral(SolverState &state, BooleanLiteral l, std::vector<std::vector<BooleanLiteral>> &added_clauses);
  std::vector<BooleanLiteral> variables_;
  std::vector<BooleanLiteral> added_lits_;
  std::vector<BooleanLiteral> previous_added_lits_;
//  std::vector<std::vector<BooleanLiteral>> added_clauses_;
  int max_;
};
}

#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_ENCODERS_INCREMENTAL_SEQUENTIAL_ENCODER_H_
