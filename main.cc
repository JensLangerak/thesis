#include <iostream>
#include "src/parser.h"

int main() {
  simple_sat_solver::Solver *s = simple_sat_solver::Parser::ReadFile("../data/example.txt");

  std::cout<<std::endl << std::endl;
  s->PrintProblem();
  std::cout<<std::endl << std::endl;
  s->PrintAssinments();
  std::cout<<std::endl << std::endl;
  s->PrintFilledProblem();
  return 0;
}
