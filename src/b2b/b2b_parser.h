//
// Created by jens on 18-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_B2B_PARSER_H_
#define SIMPLESATSOLVER_SRC_B2B_PARSER_H_

#include "b2b.h"
#include <fstream>
#include <string>
namespace simple_sat_solver::b2b {
class B2bParser {
public:
  static B2B Parse(std::string path);

private:
  explicit B2bParser(std::ifstream *stream);

  B2B RetrieveProblem();

  std::ifstream *stream_;
  int nr_meetings_;
  bool read_business_, read_nr_meetings_, read_tables_, read_slots_,
      read_morning_slots_, read_request_, read_forbidden_;
  B2B problem_;
  void ReadRequested();
  void ReadForbidden();
  int ReadIntAssignment();
  int ParseIntAssignment(bool &read);
  void ReadUntil(char i);
  bool AllIsRead();
};
} // namespace simple_sat_solver::b2b

#endif // SIMPLESATSOLVER_SRC_B2B_PARSER_H_
