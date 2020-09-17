#include "src/solver/parser.h"
#include <filesystem>
#include <iostream>

int testDir(std::string path, bool result) {
  std::cout <<path << " ---  " << (result ? "SAT" : " UNSAT") << std::endl;
  int solved = 0;
  int total = 0;
  simple_sat_solver::solver::DimacsFileParser parser = simple_sat_solver::solver::DimacsFileParser();
  for (const auto & entry : std::filesystem::directory_iterator(path)) {
    ++total;
    //std::cout<<"Solving " << entry.path() << std::endl;
    simple_sat_solver::solver::Solver *s = parser.Parse(entry.path());
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
  simple_sat_solver::solver::Solver *s =
      simple_sat_solver::solver::DimacsFileParser().Parse(file);

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
#if defined(__CYGWIN__)
  std::string baseDir = "C:/Users/jens/Downloads/sat_problems/";
#else
  std::string baseDir = "/home/jens/Downloads/sat_problems/";
#endif
  // datasets are retrieved from https://www.cs.ubc.ca/~hoos/SATLIB/benchm.html
  //testFile("../data/example2_dimacs.txt");

  testDir(baseDir + "uf20-91", true);

//  testDir(baseDir + "uf50-218", true);
//  testDir(baseDir + "uuf50-218", false);
//  testDir(baseDir + "uf75-325", true);
//  testDir(baseDir + "uuf75-325", false);
//  testDir(baseDir + "uf100-430", true);
//  testDir(baseDir + "uuf100-430", false);
//  testDir(baseDir + "uf125-538", true);
//  testDir(baseDir + "uuf125-538", false);
//
//  testDir(baseDir + "uf150-645", true);
//  testDir(baseDir + "uuf150-645", false);
//  testDir(baseDir + "uf175-753", true);
//  testDir(baseDir + "uuf175-753", false);
//  testDir(baseDir + "uf200-860", true);
//  testDir(baseDir + "uuf200-860", false);
//  testDir(baseDir + "uf225-960", true);
//  testDir(baseDir + "uuf225-960", false);
//  testDir(baseDir + "uf250-1065", true);
//  testDir(baseDir + "uuf250-1065", false);
  //testFile(baseDir + "uuf50-218/uuf50-0154.cnf");
  return 0;
}
