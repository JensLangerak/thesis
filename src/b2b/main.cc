//
// Created by jens on 18-09-20.
//
#include <iostream>
#include <string>

#include "b2b_parser.h"
namespace simple_sat_solver::b2b {
  void TestFile(std::string path) {
    B2B problem = B2bParser::Parse(path);
  }
}

int main() {
  simple_sat_solver::b2b::TestFile("../../../data/b2b/b2bsat/dzn/forum-14.dzn");
}