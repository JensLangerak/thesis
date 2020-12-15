//
// Created by jens on 18-09-20.
//
#include <assert.h>
#include <filesystem>
#include <iostream>
#include <string>

#include "../pumpkin/Propagators/Dynamic/Encoders/propagator_encoder.h"
#include "../pumpkin/Propagators/Dynamic/Encoders/totaliser_encoder.h"
#include "../solver_wrappers/pumpkin.h"
#include "../solver_wrappers/simple_solver.h"
#include "b2b_converter.h"
#include "b2b_parser.h"

namespace simple_sat_solver::b2b {

solver_wrappers::ISolver * CreateSolver() {
  return new solver_wrappers::Pumpkin(new Pumpkin::PropagatorEncoder<Pumpkin::CardinalityConstraint>::Factory(), 1000);
}

bool Test(std::string path, solver_wrappers::ISolver * solver) {
  B2B problem = B2bParser::Parse(path);
  B2bConverter converter(problem);
  sat::SatProblem sat = converter.ToSat();
  bool res = solver->Optimize(sat);
//  bool res = solver->Solve(sat);
  assert(res);
  if (res) {
    auto sol = converter.DecodeSolution(solver->GetSolution());
  }
}

bool TestFile(std::string path) {
  std::cout << "Test: " << path << std::endl;
  std::clock_t start = std::clock();

  ::Pumpkin::IEncoder<::Pumpkin::CardinalityConstraint>::IFactory * encoder_factory = new ::Pumpkin::TotaliserEncoder::Factory();
  encoder_factory->add_dynamic_ = false;
  solver_wrappers::ISolver * solver = new solver_wrappers::Pumpkin(encoder_factory, 1000);
  Test(path, solver);
  double duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  std::cout<<"Encoding: "<< duration <<'\n';
  delete solver;

//  start = std::clock();
//  encoder_factory = new ::Pumpkin::PropagatorEncoder::Factory();
//  encoder_factory->add_dynamic_ = false;
//  solver = new solver_wrappers::Pumpkin(encoder_factory);
//  Test(path, solver);
//  duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
//  std::cout<<"Propagator: "<< duration <<'\n';
//  delete solver;


  start = std::clock();
  encoder_factory = new ::Pumpkin::TotaliserEncoder::Factory();
  encoder_factory->add_dynamic_ = true;
  solver = new solver_wrappers::Pumpkin(encoder_factory, 1000);
  Test(path, solver);
  duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  std::cout<<"Dynamic: "<< duration <<'\n';
  delete solver;

  return true;
//  return solver->Solve(sat);
}
void TestDir(std::string path) {
  std::cout << "Test dir: " << path << std::endl;
  int total = 0;
  int solved = 0;
  for (const auto &b2b_entry : std::filesystem::directory_iterator(path)) {
    // TODO figure out what the morningslots array is (in similar problems that
    // array is called nMeetingsBusiness)
    if (b2b_entry.path() == "../../../data/b2b/b2bsat/dzn/forumt-14crafe.dzn")
      continue;
    //TODO update parser to handle these files
    if (b2b_entry.path() == "../../../data/b2b/b2bV2/dzn/extra" || b2b_entry.path() == "../../../data/b2b/b2bV2/dzn/taules")
      continue;
    if (!TestFile(b2b_entry.path())) {
      std::cout << path << " Unsolvable" << std::endl;
    } else {
      ++solved;
    }
    ++total;
  }
  std::cout << "Solved: " << solved << " / " << total << std::endl;
}
void TestSingleFile(std::string path) {
  std::cout << "Test: " << path << std::endl;
  B2B problem = B2bParser::Parse(path);
  B2bConverter converter(problem);
  sat::SatProblem sat = converter.ToSat();
  solver_wrappers::ISolver *solver = CreateSolver();
  bool res = solver->Solve(sat);
  if (res) {
    std::vector<int> schedule = converter.DecodeSolution(solver->GetSolution());
    std::cout << "Solved" << std::endl;
  } else {
    std::cout << "Unsolvable" << std::endl;
  }
}
} // namespace simple_sat_solver::b2b

int main() {
  std::clock_t start = std::clock();
  simple_sat_solver::b2b::TestDir("../../../data/b2b/b2bV2/dzn/");
  simple_sat_solver::b2b::TestDir("../../../data/b2b/b2bsat/dzn/");
  double duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  std::cout<<"printf: "<< duration <<'\n';
//    simple_sat_solver::b2b::TestSingleFile("../../../data/b2b/b2bsat/dzn/ticf-13crafa.dzn");
//        "../../../data/b2b/b2bsat/dzn/forum-14.dzn");
}