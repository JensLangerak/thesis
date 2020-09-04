//
// Created by jens on 04-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_PARSER_H_
#define SIMPLESATSOLVER_SRC_PARSER_H_

#include <string>
#include "solver.h"
namespace simple_sat_solver {
class Parser {
 public:
  static Solver *ReadFile(const std::string path);
  static Solver *Dimacs(const std::string path);
};
}
#endif //SIMPLESATSOLVER_SRC_PARSER_H_
