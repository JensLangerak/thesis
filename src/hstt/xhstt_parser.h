//
// Created by jens on 09-11-20.
//

#ifndef SIMPLESATSOLVER_SRC_HSTT_XHSTT_PARSER_H_
#define SIMPLESATSOLVER_SRC_HSTT_XHSTT_PARSER_H_

#include "../../lib/tinyxml2/tinyxml2.h"
#include "hstt.h"
#include <string>
namespace simple_sat_solver::hstt {
using namespace tinyxml2;
class XhsttParser {
public:
 XhsttParser(std::string path);
 std::string GetArchiveName() {return archive_name_;};
 int GetNrInstances() { return nr_instances_;};
 Hstt ParseNextInstance();
 bool HasNextInstance() {return next_problem != NULL;};
 private:
  XMLDocument doc_;
  XMLElement *root_;
  XMLElement *next_problem;
  std::string archive_name_;
  int nr_instances_;

  const char * kHighSchoolTimeTableArchive = "HighSchoolTimetableArchive";
  const char * kInstances = "Instances";
  const char * kInstance = "Instance";
  const char * kId = "Id";
  const char * kMetaData = "MetaData";
  const char * kName = "Name";
  const char * kTimes = "Times";
  const char * kTimeGroups = "TimeGroups";
  const char * kTimeGroup= "TimeGroup";
  const char * kDay = "Day";
  const char * kWeek = "Week";
  const char * kTime = "Time";
  const char * kReference = "Reference";
  const char * kResources = "Resources";
  const char * kResourceTypes = "ResourceTypes";
  const char * kResourceType = "ResourceType";
  const char * kResourceGroups = "ResourceGroups";
  const char * kResource = "Resource";
  const char * kEvents = "Events";
  const char * kEventGroups = "EventGroups";
  const char * kEventGroup = "EventGroup";
  const char * kCourse = "Course";
  const char * kEvent = "Event";
  const char * kDuration = "Duration";
  const char * kRole = "Role";
  const char * kWorkload = "Workload";

  void ParseTimeGroups(Hstt &hstt);
  void ParseTimes(Hstt &hstt);
  void ParseResourceTypes(Hstt &hstt);
  void ParseResourceGroups(Hstt &hstt);
  void ParseResources(Hstt &hstt);
  void ParseEventGroups(Hstt &hstt);
  void ParseEvents(Hstt &hstt);
};

}

#endif // SIMPLESATSOLVER_SRC_HSTT_XHSTT_PARSER_H_
