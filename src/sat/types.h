//
// Created by jens on 18-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_SAT_TYPES_H_
#define SIMPLESATSOLVER_SRC_SAT_TYPES_H_
#include <vector>
namespace simple_sat_solver::sat {
typedef int Var;

struct Lit {
  Var x;
  bool complement;
  Lit(Var x, bool complement) : x(x), complement(complement){};
  Lit() : x(0), complement(false){};
  Lit(Var x) : x(x), complement(false) {};
};

inline Lit operator~(Lit p) { return Lit(p.x, !p.complement); }
inline bool operator==(Lit p, Lit q) {
  return p.x == q.x && p.complement == q.complement;
}
inline bool operator!=(Lit p, Lit q) {
  return p.x != q.x || p.complement != q.complement;
}



} // namespace simple_sat_solver::sat
#endif // SIMPLESATSOLVER_SRC_SAT_TYPES_H_
