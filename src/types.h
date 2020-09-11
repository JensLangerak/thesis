//
// Created by jens on 03-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_TYPES_H_
#define SIMPLESATSOLVER_SRC_TYPES_H_

#include <queue>
#include <vector>
namespace simple_sat_solver {
typedef int Var;

template <typename T> using Vec = std::vector<T>;
template <typename T> using Queue = std::queue<T>;

enum class LBool { kUnknown, kTrue, kFalse };

inline LBool operator~(LBool x) {
  switch (x) {
  case LBool::kUnknown:
    return LBool::kUnknown;
  case LBool::kTrue:
    return LBool::kFalse;
  case LBool::kFalse:
    return LBool::kTrue;
  }
  return LBool::kUnknown;
}
struct Lit {
  Var x;
  bool complement;
  Lit(Var x, bool complement) : x(x), complement(complement){};
  Lit() : x(0), complement(false){};
};

inline Lit operator~(Lit p) {
  return Lit(p.x, !p.complement);
}
inline bool operator==(Lit p, Lit q) {
  return p.x == q.x && p.complement == q.complement;
}
inline bool operator!=(Lit p, Lit q) {
  return p.x != q.x || p.complement != q.complement;
}
} // namespace simple_sat_solver
#endif // SIMPLESATSOLVER_SRC_TYPES_H_
