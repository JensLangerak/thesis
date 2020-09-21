//
// Created by jens on 18-09-20.
//

#include "b2b_parser.h"
#include <fstream>
#include <iostream>
namespace simple_sat_solver::b2b {
B2B B2bParser::Parse(std::string path) {
  std::ifstream file_stream(path);
  if (!file_stream.is_open()) {
    std::cout << "Cannot open file: " << path << std::endl;
    file_stream.close();
    throw "Cannot open file";
  }
  B2bParser reader(&file_stream);

  B2B problem = reader.RetrieveProblem();
  return problem;
}
B2bParser::B2bParser(std::ifstream *stream)
    : stream_(stream), problem_(), read_business_(false),
      read_forbidden_(false), read_morning_slots_(false), read_request_(false),
      read_slots_(false), read_tables_(false), read_nr_meetings_(false) {}

B2B B2bParser::RetrieveProblem() {
  std::string word;
  while ((*stream_) >> word) {
    if (word == "nBusiness") {
      problem_.nr_participants = ParseIntAssignment(read_business_);
    } else if (word == "nMeetings") {
      nr_meetings_ = ParseIntAssignment(read_nr_meetings_);
    } else if (word == "nTables") {
      problem_.nr_locations = ParseIntAssignment(read_tables_);
    } else if (word == "nTotalSlots") {
      problem_.nr_timeslots = ParseIntAssignment(read_slots_);
    } else if (word == "nMorningSlots") {
      problem_.nr_morning_slots = ParseIntAssignment(read_morning_slots_);
    } else if (word == "requested") {
      ReadRequested();
    } else if (word == "forbidden") {
      ReadForbidden();
    } else if (word == "nMeetingsBusiness" || word == "fixat") {
      ReadUntil(';');
    } else {
      throw "Unknown option";
    }
  }
  if (!AllIsRead())
    throw "Missing data";
  return problem_;
}

int B2bParser::ParseIntAssignment(bool &read) {
  if (read)
    throw "Already read";
  read = true;

  return ReadIntAssignment();
}
void B2bParser::ReadRequested() {
  if (!read_nr_meetings_)
    throw "Does not know how many meetings to read";
  if (read_request_)
    throw "Already read meetings";
  read_request_ = true;
  for (int i = 0; i < nr_meetings_; i++) {
    ReadUntil('|');
    Meeting meeting;
    int value;
    *stream_ >> value;
    meeting.p1 = value-1;
    std::string read;
    *stream_ >> read;
    if (read != ",")
      throw "Unexpected char";
    *stream_ >> value;
    meeting.p2 = value-1;
    *stream_ >> read;
    if (read != ",")
      throw "Unexpected char";
    int time_of_day;
    *stream_ >> time_of_day;
    if (time_of_day == 1)
      meeting.time_of_day = TimeOfDay::kMorning;
    else if (time_of_day == 2)
      meeting.time_of_day = TimeOfDay::kAfternoon;
    else
      meeting.time_of_day = TimeOfDay ::kDontCare;

    problem_.meetings.push_back(meeting);
  }

  ReadUntil('|');
  std::string end;
  *stream_ >> end;
  if (end != "];")
    throw "Something went wrong";
}
void B2bParser::ReadForbidden() {
  if (!read_business_)
    throw "Does not know the number of participants";
  if (read_forbidden_)
    throw "Already read forbidden";
  read_forbidden_ = true;
  ReadUntil('[');
  for (int i = 0; i < problem_.nr_participants; i++) {
    std::vector<int> forbidden_slots;
    ReadUntil('{');
    bool stop = false;
    while (!stop) {
      char buffer[10] = {0};
      int index = -1;
      do {
        ++index;
        *stream_ >> buffer[index];
        if (buffer[index] == '}') {
          stop = true;
          break;
        }
      } while (buffer[index] != ',');

      if (stop)
        break;
      buffer[index] = 0;
      int forbidden = std::stoi(buffer) - 1;
      if (forbidden >= 0)
        forbidden_slots.push_back(forbidden);
    }
    problem_.forbidden_slots.push_back(forbidden_slots);
  }
  ReadUntil(';');
}
int B2bParser::ReadIntAssignment() {
  std::string read;
  *stream_ >> read; // =
  if (read != "=")
    throw "Unexpected char";
  int result;
  *stream_ >> result;
  *stream_ >> read; // ;
  if (read != ";")
    throw "Unexpected char";
  return result;
}
void B2bParser::ReadUntil(char i) {
  char r;
  while (*stream_ >> r) {
    if (r == i)
      return;
  }
}
bool B2bParser::AllIsRead() {
  return read_forbidden_ && read_business_ && read_nr_meetings_ &&
         read_morning_slots_ && read_slots_ && read_tables_ && read_request_;
}

} // namespace simple_sat_solver::b2b