//
// Created by jens on 11-11-20.
//

#include "parser.h"
#include <fstream>
#include <iostream>
#include <sstream>
namespace simple_sat_solver::ctt {
Ctt Parser::Parse(std::string path) {
  std::ifstream file_stream(path);
  if (!file_stream.is_open()) {
    std::cout << "Cannot open file: " << path << std::endl;
    file_stream.close();
    throw "Cannot open file";
  }
  Parser parser(&file_stream);
  parser.ReadHeader();
  parser.ReadSection("COURSES:", parser.problem.nr_courses,&Parser::ReadCourseLine);
  parser.ReadSection("ROOMS:", parser.problem.nr_rooms,&Parser::ReadRoomLine);
  parser.ReadSection("CURRICULA:", parser.problem.nr_curricula,&Parser::ReadCurriculumLine);
  parser.ReadSection("UNAVAILABILITY_CONSTRAINTS:", parser.problem.nr_constraints,&Parser::ReadUnavailabilityConstraintLine);

  parser.ValidateSectionHeader("END.");
  return parser.problem;
}
void Parser::ReadHeader() {
  problem.name = ReadStringFromHeader("Name:");
  problem.nr_courses = ReadIntFromHeader("Courses:");
  problem.nr_rooms = ReadIntFromHeader("Rooms:");
  problem.nr_days = ReadIntFromHeader("Days:");
  problem.nr_periods_per_day = ReadIntFromHeader("Periods_per_day:");
  problem.nr_curricula = ReadIntFromHeader("Curricula:");
  problem.nr_constraints = ReadIntFromHeader("Constraints:");
}

std::string Parser::ReadStringFromHeader(std::string data_name) {
  std::istringstream iss = GetLineStream(data_name);

  std::string word;
  iss >> word;
  return word;
}
std::istringstream Parser::GetLineStream() {
  std::string line = ReadNextLine();
  std::istringstream iss(line);
  return iss;
}
std::istringstream Parser::GetLineStream(std::string data_name) {
  std::istringstream iss = GetLineStream();
  std::string word;
  iss >> word;
  if (word != data_name)
    throw "Unexpected format";
  return iss;
}
int Parser::ReadIntFromHeader(std::string data_name) {
  std::istringstream iss = GetLineStream(data_name);

  int data;
  iss >> data;
  return data;
}

void Parser::ValidateSectionHeader(std::string header) {
  std::string line = ReadNextLine();
  if (line.empty())
    line = ReadNextLine();
  if (line != header)
    throw "Unexpected format";
}
void Parser::ReadCourseLine() {
  Course course;
  course.index = problem.courses.size();

  std::istringstream iss = GetLineStream();
  iss >> course.id;

  std::string teacher_id;
  iss >> teacher_id;
  course.teacher_index = GetTeacher(teacher_id);
  iss >> course.nr_lectures;
  iss >> course.min_working_days;
  iss >> course.nr_students;

  problem.courses.push_back(course);
}
std::string Parser::ReadNextLine() {
  std::string line;
  std::getline(*fstream, line);
  if (line.size() > 0 && line.back() == '\r') // on linux it splits on \n while file uses \r\n , so remove \r
    line.erase(line.size() - 1);
  return line;
}
 int Parser::GetTeacher(std::string teacher_id) {
  int teacher = problem.FindId(problem.teachers, teacher_id);
  if (teacher < 0) {
    Teacher t;
    t.index = problem.teachers.size();
    t.id = teacher_id;
    problem.teachers.push_back(t);
    teacher = t.index;
  }
  return teacher;
}

void Parser::ReadRoomLine() {
  Room room;
  room.index = problem.rooms.size();

  std::istringstream iss = GetLineStream();
  iss >> room.id;
  iss >> room.max_capactity;
  problem.rooms.push_back(room);
}
void Parser::ReadSection(std::string section_title, int nr_lines, void (Parser::*f_read_line)()) {
  ValidateSectionHeader(section_title);
  for (int i = 0; i < nr_lines; ++i) {
    (this->*f_read_line)();
  }
}
void Parser::ReadCurriculumLine() {
  Curriculum curriculum;
  curriculum.index= problem.curricula.size();

  std::istringstream iss = GetLineStream();
  iss >> curriculum.id;
  iss >> curriculum.nr_courses;

  for (int i = 0; i < curriculum.nr_courses; ++i) {
    std::string course_id;
    iss >> course_id;
    int course = problem.FindId(problem.courses, course_id);
    if (course < 0)
      throw "Course not found";
    curriculum.courses_indices.push_back(course);
  }
  problem.curricula.push_back(curriculum);
}
void Parser::ReadUnavailabilityConstraintLine() {
  UnavailabilityConstraint c;
  c.index = problem.curricula.size();

  std::istringstream iss = GetLineStream();
  std::string course_id;
  iss >> course_id;
  int course = problem.FindId(problem.courses, course_id);
  if (course < 0)
    throw "Course not found";

  c.course_index = course;
  iss >> c.day;
  iss >> c.period;
  if (c.day >= problem.nr_days || c.period >= problem.nr_periods_per_day)
    throw "Illegal day or period";
  problem.constraints.push_back(c);
}
} // namespace simple_sat_solver::ctt