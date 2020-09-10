#include <iostream>
#include <filesystem>
#include "src/parser.h"


int testDir(std::string path, bool result) {
  std::cout <<path << " ---  " << (result ? "SAT" : " UNSAT") << std::endl;
  int solved = 0;
  int total = 0;
  for (const auto & entry : std::filesystem::directory_iterator(path)) {
    ++total;
    //std::cout<<"Solving " << entry.path() << std::endl;
    simple_sat_solver::Solver *s = simple_sat_solver::Parser::Dimacs(entry.path());
    if (s->Solve() == result) {
      //std::cout << "Solved" <<std::endl;
      ++solved;
    } else {
      std::cout << "Wrong: " << entry.path() << std::endl;
    }
    delete s;
  }
  std::cout << "Solved/found unsat " << solved << " out of " << total << std::endl;

  return 0;
}
int testFile(std::string file) {
  simple_sat_solver::Solver *s = simple_sat_solver::Parser::Dimacs(file);

  s->PrintProblem();
  std::cout<<std::endl << std::endl;
  bool res = s->Solve();
  if (res) {
    s->PrintAssignments();
    std::cout << std::endl << std::endl;
    s->PrintFilledProblem();
  } else {
    s->PrintAssignments();
    std::cout << "Cannot be solved" << std::endl;
  }
  return 0;
}
int main() {
  //testFile("../data/example2_dimacs.txt");
  testDir("/home/jens/Downloads/sat_problems/uf20-91", true);
  testDir("/home/jens/Downloads/sat_problems/uf50-218", true);
  testDir("/home/jens/Downloads/sat_problems/uuf50-218", false);
  testDir("/home/jens/Downloads/sat_problems/uf75-325", true);
  testDir("/home/jens/Downloads/sat_problems/uuf75-325", false);
  testDir("/home/jens/Downloads/sat_problems/uf100-430", true);
  testDir("/home/jens/Downloads/sat_problems/uuf100-430", false);
  testDir("/home/jens/Downloads/sat_problems/uf125-538", true);
  testDir("/home/jens/Downloads/sat_problems/uuf125-538", false);
  //testFile("/home/jens/Downloads/uf20-91/uf20-0716.cnf");
  return 0;
}
