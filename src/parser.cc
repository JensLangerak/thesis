//
// Created by jens on 04-09-20.
//

#include <fstream>
#include <iostream>
#include <sstream>
#include "parser.h"
namespace simple_sat_solver {
//TODO fault handling and closing
Solver *Parser::ReadFile(const std::string path) {
  std::ifstream satFile(path);
  if (satFile.is_open()) {
    Solver *s = new Solver();
    std::string line;

    // get number of vars
    if (!getline(satFile, line))
      return nullptr;
    int nrVars = std::stoi(line);
    for (int i = 0; i < nrVars; i++)
      s->NewVar();

    //empty line
    getline(satFile, line);

    std::string lit;
    Vec<Lit> clause;
    while(getline(satFile, line)) {
      clause.clear();
      std::istringstream sstream(line);
      while (sstream >> lit) {
        Lit l;
        l.complement = false;
        if (lit[0] == '~') {
          l.complement = true;
          lit.erase(0,1);
        }
        l.x = std::stoi(lit);
        if (l.x >= nrVars) {
          std::cout << std::endl << "ID is higher than expected " << std::endl;
          delete s;
          return nullptr;
        }
        clause.push_back(l);
      }

      s->AddClause(clause);

    }
    satFile.close();
    return s;
  } else {
    std::cout << "Cannot open file: " << path << std::endl;
    return nullptr;
  }
}
}
