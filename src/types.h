//
// Created by jens on 03-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_TYPES_H_
#define SIMPLESATSOLVER_SRC_TYPES_H_

#include <vector>
#include <queue>
// TODO currently it are mainly placeholders for the actual types.
namespace simple_sat_solver {
typedef int Var; // TODO not sure if this is the same
template<typename T> using Vec = std::vector<T>;
template<typename T> using Queue = std::queue<T>;

struct Lit {
  Var x;
};

// TODO
Lit operator ~(Lit p) { return p; }
bool Sign(Lit p) { return true; }
int GetVar(Lit p) { return p.x; } // is var in the paper
int Index(Lit p) { return p.x; }

enum class LBool { kUnknown, kTrue, kFalse};

LBool operator ~(LBool x) {
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
}
#endif //SIMPLESATSOLVER_SRC_TYPES_H_
