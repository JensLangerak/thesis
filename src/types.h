//
// Created by jens on 03-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_TYPES_H_
#define SIMPLESATSOLVER_SRC_TYPES_H_

#include <vector>
#include <queue>
// TODO currently it are mainly placeholders for the actual types.
namespace simple_sat_solver {
class Constr;
typedef int Var;
template<typename T> using Vec = std::vector<T>;
template<typename T> using Queue = std::queue<T>;


enum class LBool { kUnknown, kTrue, kFalse};

inline LBool operator ~(LBool x) {
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
};

// TODO
inline Lit operator ~(Lit p) { Lit q; q.x = p.x; q.complement = !p.complement; return q;}
inline bool operator ==(Lit p, Lit q) { return p.x == q.x && p.complement == q.complement;}
inline bool operator !=(Lit p, Lit q) { return p.x != q.x || p.complement != q.complement;}
inline bool Sign(Lit p) { return p.complement; }
inline int GetVar(Lit p) { return p.x; } // is var in the paper
inline int Index(Lit p) { return p.x; }
}
#endif //SIMPLESATSOLVER_SRC_TYPES_H_
