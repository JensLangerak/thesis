//
// Created by jens on 09-11-20.
//
#include "xhstt_parser.h"
#include <iostream>
namespace simple_sat_solver::hstt {
  void test() {

//    XhsttParser parser = XhsttParser("/home/jens/Downloads/XHSTT-2014.xml");
    XhsttParser parser = XhsttParser("/home/jens/Downloads/BrazilInstance2.xml");
   auto test = parser.ParseNextInstance();
   int brealp;
   brealp ++;
  }
}

int main() {
  std::cout << "test" <<std::endl;
  simple_sat_solver::hstt::test();
}