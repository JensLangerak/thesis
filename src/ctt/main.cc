//
// Created by jens on 11-11-20.
//
#include "../pumpkin/Propagators/Cardinality/Encoders/i_encoder.h"
#include "../pumpkin/Propagators/Cardinality/Encoders/totaliser_encoder.h"
#include "../solver_wrappers/i_solver.h"
#include "../solver_wrappers/pumpkin.h"
#include "ctt.h"
#include "ctt_converter.h"
#include "parser.h"
#include <iostream>
namespace simple_sat_solver::ctt {
  void Test(std::string file) {
    ::Pumpkin::IEncoder::IFactory * encoder_factory = new ::Pumpkin::TotaliserEncoder::Factory();
    encoder_factory->add_dynamic_ = false;
    solver_wrappers::ISolver * solver = new solver_wrappers::Pumpkin(encoder_factory);

    Ctt problem = Parser::Parse(file);
    CttConverter converter(problem);
    sat::SatProblem sat = converter.GetSatProblem();
//    bool res = solver->Optimize(sat);
  bool res = solver->Solve(sat);
    assert(res);
    if (res) {
      auto sol = converter.ConvertSolution(solver->GetSolution());
      converter.PrintSolution(sol);
      std::ofstream outfile ("../../../data/ctt/solution.txt");
      converter.PrintSolution(sol, outfile);
      outfile.close();
      std::cout << "Penalty: " << converter.ValidateSolution(sol);
    }
  }
}

int main() {

  std::cout << "Hello world" << std::endl;
  simple_sat_solver::ctt::Test("../../../data/ctt/toyexample.ctt");
}