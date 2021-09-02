//
// Created by jens on 09-11-20.
//

#include "xhstt_parser.h"
#include "../../lib/tinyxml2/tinyxml2.h"
#include <cassert>
namespace simple_sat_solver::hstt {
using namespace tinyxml2;
XhsttParser::XhsttParser(std::string path) {
  doc_.LoadFile(path.c_str());
  root_ = doc_.FirstChildElement();
  auto test = root_->Name();
  assert(strcmp(root_->Name(), kHighSchoolTimeTableArchive) == 0);
//  archive_name_ = root_->Attribute(kId);
  nr_instances_ = 0;
  for (auto child = root_->FirstChildElement(kInstances)->FirstChildElement();
       child; child = child->NextSiblingElement()) {
    assert(strcmp(child->Name(), kInstance) ==0);
    nr_instances_++;
  }
  next_problem = root_->FirstChildElement(kInstances)->FirstChildElement();
}
Hstt XhsttParser::ParseNextInstance() {
  Hstt res;
  res.name_ = next_problem->FirstChildElement(kMetaData)->FirstChildElement(kName)->GetText();
  ParseTimeGroups(res);
  ParseTimes(res);
  ParseResourceTypes(res);
  ParseResourceGroups(res);
  ParseResources(res);
  ParseEventGroups(res);
  ParseEvents(res);
  ParseConstraints(res);

  return res;
}
void XhsttParser::ParseTimeGroups(Hstt &hstt) {
  auto times = next_problem->FirstChildElement(kTimes);
  auto time_groups = times->FirstChildElement(kTimeGroups);
  for (auto child = time_groups->FirstChildElement(); child; child = child->NextSiblingElement()) {
    if (strcmp(child->Name() , kDay) == 0)
      hstt.days_.push_back(child->Attribute(kId));
    else if (strcmp(child->Name() , kWeek) == 0)
      hstt.weeks_.push_back(child->Attribute(kId));
    else if (strcmp(child->Name() , kTimeGroup) == 0)
      hstt.time_groups_.push_back(child->Attribute(kId));
    else
      throw "Error";
  }
}
void XhsttParser::ParseTimes(Hstt &hstt) {
  auto times = next_problem->FirstChildElement(kTimes);
  for (auto child = times->FirstChildElement(kTime); child; child = child->NextSiblingElement(kTime)) {
    Hstt::Time time;
    time.id = child->Attribute(kId);
    auto day = child->FirstChildElement(kDay);
    auto week = child->FirstChildElement(kWeek);
    auto timegroups = child->FirstChildElement(kTimeGroups);
    if (day)
      time.day = hstt.FindId(hstt.days_, day->Attribute(kReference));
    if (week)
      time.week = hstt.FindId(hstt.weeks_, week->Attribute(kReference));
    if (timegroups) {
      for (auto timeGroup = timegroups->FirstChildElement(); timeGroup; timeGroup = timeGroup->NextSiblingElement()) {
        time.time_groups.push_back(hstt.FindId(hstt.time_groups_, timeGroup->Attribute(kReference)));
      }
    }
    hstt.times_.push_back(time);
  }
}
void XhsttParser::ParseResourceTypes(Hstt &hstt) {
  auto resourceTypes = next_problem->FirstChildElement(kResources)->FirstChildElement(kResourceTypes);
  for (auto type = resourceTypes->FirstChildElement(); type; type = type->NextSiblingElement())
    hstt.resource_types_.push_back(type->Attribute(kId));
}
void XhsttParser::ParseResourceGroups(Hstt &hstt) {

  auto resourcegroups = next_problem->FirstChildElement(kResources)->FirstChildElement(kResourceGroups);
  for (auto type = resourcegroups->FirstChildElement(); type; type = type->NextSiblingElement()) {
    Hstt::ResourceGroup group;
    group.id = type->Attribute(kId);
    group.resource_type = hstt.FindId(hstt.resource_types_, type->FirstChildElement(kResourceType)->Attribute(kReference));
  hstt.resource_groups_.push_back(group);
  }


}
void XhsttParser::ParseResources(Hstt &hstt) {
  auto resources = next_problem->FirstChildElement(kResources);
  for (auto resourceElement = resources->FirstChildElement(kResource); resourceElement; resourceElement = resourceElement->NextSiblingElement(kResource)) {
    Hstt::Resource resource;
    resource.id = resourceElement->Attribute(kId);
    resource.resource_type = hstt.FindId(hstt.resource_types_, resourceElement->FirstChildElement(kResourceType)->Attribute(kReference));
    auto groups = resourceElement->FirstChildElement(kResourceGroups);
    for (auto group = groups->FirstChildElement(); group; group = group->NextSiblingElement()) {
      auto group_id = hstt.FindId<Hstt::ResourceGroup>(hstt.resource_groups_, group->Attribute(kReference), [](auto rg) -> auto { return rg.id;});
//auto ref = group->Attribute(kReference);
//auto group_id = hstt.FindResourceGroup(ref);
      resource.resource_groups.push_back(group_id);
    }
    hstt.resources_.push_back(resource);
  }

}
void XhsttParser::ParseEventGroups(Hstt &hstt) {
  auto groups = next_problem->FirstChildElement(kEvents)->FirstChildElement(kEventGroups);
  for (auto group = groups->FirstChildElement(); group; group = group->NextSiblingElement()) {
    std::string id = group->Attribute(kId);
    if (strcmp(group->Name(), kCourse) == 0)
      hstt.courses_.push_back(id);
    else if (strcmp(group->Name(), kEventGroup) == 0)
      hstt.event_groups_.push_back(id);
    else
      throw "Error";
  }

}
void XhsttParser::ParseEvents(Hstt &hstt) {
  auto events = next_problem->FirstChildElement(kEvents)->FirstChildElement(kEvent);
  for (auto event_element = events; event_element; event_element = event_element->NextSiblingElement()) {
    Hstt::Event event;
    event.id = event_element->Attribute(kId);
    event.duration = event_element->FirstChildElement(kDuration)->IntText();
    auto workload = event_element->FirstChildElement(kWorkload);
    if (workload)
      event.workload = workload->IntText();
    else
      event.workload = event.duration;
    auto course = event_element->FirstChildElement(kCourse);
    if (course)
      event.course = hstt.FindId(hstt.courses_, course->Attribute(kReference));
    auto time = event_element->FirstChildElement(kTime);
    if (time) {
      auto time_id = hstt.FindId<Hstt::Time>(
          hstt.times_, time->Attribute(kReference),
          [](auto t) -> auto { return t.id; });
      event.time = time_id;
    }
    auto resources = event_element->FirstChildElement(kResources);
    if (resources) {
      for (auto resource = resources->FirstChildElement(); resource;
           resource = resource->NextSiblingElement()) {
        Hstt::EventResource event_resource;
        if (resource->FindAttribute(kReference)) {
          std::string reference = resource->Attribute(kReference);
          if (!reference.empty()) {
            auto ref_id = hstt.FindId<Hstt::Resource>(
                hstt.resources_, reference,
                [](auto t) -> auto { return t.id; });
            event_resource.resource = ref_id;
          }
        }
        auto role = resource->FirstChildElement(kRole);
        if (role)
          event_resource.id = role->GetText();
        auto type = resource->FirstChildElement(kResourceType);
        if (type)
          event_resource.resource_type =
              hstt.FindId(hstt.resource_types_, type->Attribute(kReference));

        event.resources.push_back(event_resource);
      }
    }

    auto resource_groups = event_element->FirstChildElement(kResourceGroups);
    if (resource_groups) {
      for (auto group = resource_groups->FirstChildElement(); group;
           group = group->NextSiblingElement()) {
        auto group_id = hstt.FindId<Hstt::ResourceGroup>(hstt.resource_groups_, group->Attribute(kReference), [](auto rg) -> auto { return rg.id;});
        event.resource_groups.push_back(group_id);
      }
    }


    auto event_groups = event_element->FirstChildElement(kEventGroups);
    if (event_groups) {
      for (auto group = event_groups->FirstChildElement(); group;
           group = group->NextSiblingElement()) {
        event.event_groups.push_back(hstt.FindId(hstt.event_groups_, group->Attribute(kReference)));
      }
    }
    hstt.events_.push_back(event);
  }
}
void XhsttParser::ParseConstraints(Hstt &hstt) {
  auto constraints = next_problem->FirstChildElement(kConstraints);
  for (auto constraint = constraints->FirstChildElement(); constraint; constraint = constraint->NextSiblingElement()) {

  }

}
} // namespace simple_sat_solver::hstt
