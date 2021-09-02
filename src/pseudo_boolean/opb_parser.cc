//
// Created by jens on 25-01-21.
//

#include "opb_parser.h"
#include "../sat/constraints/pseudo_boolean_constraint.h"
#include <fstream>
#include <iostream>
#include <sstream>
namespace simple_sat_solver::pseudo_boolean {

sat::SatProblem * OpbParser::Parse(std::string path) {
  std::ifstream file_stream(path);
  if (!file_stream.is_open()) {
    std::cout << "Cannot open file: " << path << std::endl;
    file_stream.close();
    throw "Cannot open file";
  }
  OpbParser parser(&file_stream);
  parser.Parse();
  return parser.problem;
}
void OpbParser::Parse() {
  std::string line;
  bool first = true;
  while (std::getline(*fstream, line)) {
    if (line[0] == '*' || line.empty())
      continue;
    std::stringstream ss;
    ss << line;
    if (ss.peek() == 'm') {
      std::string word;
    ss >> word;
    if (word != "min:") {
      throw "what is this?";
    }
      if (!first) {
        throw "min should be the first line?";
      }
      std::vector<sat::WeightedLit> clause = read_min_line(ss);
      for (auto w : clause)
        problem->AddToMinimize(w);
    } else {
      ss.clear();
      ss << line;
     read_constraint(ss);
    }
    first = false;
  }
}
std::vector<sat::WeightedLit> OpbParser::read_min_line(std::stringstream &ss) {
  std::vector<sat::WeightedLit> res;
  int w;
  std::string label;
  std::string next;
  while (true) {
    ss >> next;
    char* p;
    w = std::strtol(next.c_str(), &p, 10);
    if (*p) { // not a string
      break;
    }
    ss >> label;
    if (w == 0)
      continue;
    res.push_back(getWeightedLit(w, label));
  }
  return res;
}
sat::WeightedLit OpbParser::getWeightedLit(int w, std::string key) {

  bool complement = key[0] == '~';
  if (complement)
    key.erase(0,1);
  if (string_lit_map.count(key) == 0) {
    sat::Lit l = problem->AddNewVar();
    string_lit_map[key] = l;
  }
  sat::Lit l = string_lit_map[key];
  if (complement)
    l=~l;
  if (w > 0) {
    return sat::WeightedLit(l, w);
  } else {
    return sat::WeightedLit(~l, -w);
  }
}
void OpbParser::read_constraint(std::stringstream &ss) {
  std::vector<sat::Lit> lits;
  std::vector<int> weights;
  int w;
  std::string label;
  std::string test;
  std::string constraint;
  int rhs_correction = 0;
  while (true) {
    ss >> constraint;
    char* p;
    w = std::strtol(constraint.c_str(), &p, 10);
    if (*p) { // not a string
      break;
    }



    ss >> label;
    if (w == 0)
      continue;
    sat::WeightedLit lit = getWeightedLit(w, label);
    lits.push_back(lit.l);
    weights.push_back(lit.w);
    if (w < 0)
      rhs_correction += lit.w;
  }

  int rhs;
  ss >> rhs;
  rhs += rhs_correction;
  int min=  0;
  int max = 0;
  for (int i = 0; i < weights.size(); ++i) {
    max += weights[i];
  }
  int weight_sum = max;

  if (constraint == "=") {
    min = rhs;
    max = rhs;
  } else if (constraint == "<=") {
    max = rhs;
  } else if (constraint == ">=") {
    min = rhs;
  } else if (constraint == "<") {
    max = rhs - 1;
  } else if (constraint == ">") {
    min = rhs + 1;
  } else {
    throw "Unknown constraint";
  }
  if (max < weight_sum) {
    if (max == 1) {
      std::vector<sat::Lit> lits_1;
      for (int i = 0; i < weights.size(); ++i) {
        if (weights[i] == 1)
          lits_1.push_back(lits[i]);
      }
      problem->AtMostOne(lits_1);
    } else if (max > 1) {
      problem->AddConstraint(
          new sat::PseudoBooleanConstraint(lits, weights, 0, max));
    } else if(max == 0){
      problem->None(lits);
    } else{
      throw "Error";
    }
  }
  if (min > 0) {
    std::vector<sat::Lit> invers_lits;
    for (sat::Lit l : lits) {
      invers_lits.push_back(~l);
    }
    int inverse_max = weight_sum - min;
    if (inverse_max == 1) {
      std::vector<sat::Lit> lits_1;
      for (int i = 0; i < weights.size(); ++i) {
        if (weights[i] == 1)
          lits_1.push_back(invers_lits[i]);
      }
      problem->AtMostOne(lits_1);
    } else if (inverse_max > 1) {
      problem->AddConstraint(
          new sat::PseudoBooleanConstraint(invers_lits, weights, 0, inverse_max));
    } else if(inverse_max == 0){
      problem->None(invers_lits);
    } else{
      throw "Error";
    }
  }
}
}
