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
Solver *Parser::Dimacs(const std::string path) {
  std::ifstream satFile(path);
  if (satFile.is_open()) {
    Solver *s = new Solver();
    std::string line;


    while(getline(satFile, line)) {
       if(line[0] == 'c')
         continue;
       else
         break;
    }

    std::string type = "p cnf ";
    if (line.rfind(type, 0) != 0)
      return nullptr;

    line.erase(0, type.length());
    std::stringstream sstream;
    sstream << line;
    int nrVars, nrClauses;
    sstream >> nrVars;
    sstream >> nrClauses;

    for (int i = 0; i < nrVars; i++)
      s->NewVar();

    Vec<Lit> clause;
    int lit;
    while(getline(satFile, line)) {
      --nrClauses;
      clause.clear();
      std::stringstream ss;
      ss << line;

      ss >> lit;
      while (lit != 0) {
        Lit l;
        l.complement = (lit < 0);
        l.x = abs(lit) - 1;
        if (l.x >= nrVars) {
          std::cout << std::endl << "ID is higher than expected " << std::endl;
          delete s;
          return nullptr;
        }
        clause.push_back(l);
        ss >> lit;
      }

      s->AddClause(clause);
      if(nrClauses == 0)
        break;
    }
    satFile.close();
    return s;
  } else {
    std::cout << "Cannot open file: " << path << std::endl;
    return nullptr;
  }
}
}
