#include <iostream>
#include <filesystem>
#include "src/parser.h"


int testDir() {
  std::string path = "/home/jens/Downloads/uf20-91";
  int solved = 0;
  int total = 0;
  for (const auto & entry : std::filesystem::directory_iterator(path)) {
    ++total;
    //std::cout<<"Solving " << entry.path() << std::endl;
    simple_sat_solver::Solver *s = simple_sat_solver::Parser::Dimacs(entry.path());
    if (s->Solve()) {
      //std::cout << "Solved" <<std::endl;
      ++solved;
    } else {
      std::cout << "Wrong result for: " << entry.path() << std::endl;
    }
    delete s;
  }
  std::cout << "Solved " << solved << " out of " << total << std::endl;

  return 0;
}
int testFile(std::string file) {
  simple_sat_solver::Solver *s = simple_sat_solver::Parser::Dimacs(file);

  s->PrintProblem();
  std::cout<<std::endl << std::endl;
  bool res = s->Solve();
  if (res) {
    s->PrintAssinments();
    std::cout << std::endl << std::endl;
    s->PrintFilledProblem();
  } else {
    std::cout << "Cannot be solved" << std::endl;
  }
  return 0;
}
int main() {
  testFile("/home/jens/Downloads/uf20-91/uf20-0799.cnf");
  //testDir();
  return 0;
}
