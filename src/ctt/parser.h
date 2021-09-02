//
// Created by jens on 11-11-20.
//

#ifndef SIMPLESATSOLVER_SRC_CTT_PARSER_H_
#define SIMPLESATSOLVER_SRC_CTT_PARSER_H_

#include "ctt.h"
#include <fstream>
namespace simple_sat_solver::ctt {
class Parser {
public:
  static Ctt Parse(std::string path);
private:
  Parser(std::ifstream *fstream) : fstream(fstream), problem() {};

  std::ifstream *fstream;
  Ctt problem;
  void ReadHeader();
  std::string ReadStringFromHeader(std::string data_name);
  std::istringstream GetLineStream(std::string data_name);
  int ReadIntFromHeader(std::string data_name);
  void ValidateSectionHeader(std::string header);
  void ReadCourseLine();
  std::string ReadNextLine();
  std::istringstream GetLineStream();
  int GetTeacher(std::string teacher_id);
  void ReadRoomLine();
  void ReadCurriculumLine();
  void ReadUnavailabilityConstraintLine();
  void ReadSection(std::string section_title, int nr_lines, void (Parser::*f_read_line)());
};
}
#endif // SIMPLESATSOLVER_SRC_CTT_PARSER_H_
