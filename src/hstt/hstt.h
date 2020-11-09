//
// Created by jens on 09-11-20.
//

#ifndef SIMPLESATSOLVER_SRC_HSTT_HSTT_H_
#define SIMPLESATSOLVER_SRC_HSTT_HSTT_H_

#include <functional>
#include <optional>
#include <string>
#include <vector>
namespace simple_sat_solver::hstt {
class Hstt {
public:
  Hstt() {};
  std::string name_;
  std::vector<std::string> time_groups_;
  std::vector<std::string> days_;
  std::vector<std::string> weeks_;

  struct Time {
    std::optional<int> day;
    std::optional<int> week;
    std::vector<int> time_groups;
    std::string id;
  };

  std::vector<Time> times_;

  std::vector<std::string> resource_types_;

  struct ResourceGroup{
    std::string id;
    int resource_type;
  };

  std::vector<ResourceGroup> resource_groups_;

  struct Resource {
    std::string id;
    int resource_type;
    std::vector<int> resource_groups;
  };
  std::vector<Resource> resources_;

  std::vector<std::string> courses_;
  std::vector<std::string> event_groups_;

  struct EventResource {
    std::optional<int> resource;
    std::optional<std::string> id;
    std::optional<int> resource_type;
  };
  struct Event {
    std::string id;
    int duration;
    std::optional<int> workload;
    std::optional<int> course;
    std::optional<int> time;
    std::vector<EventResource> resources;
    std::vector<int> resource_groups;
    std::vector<int> event_groups;
  };
  std::vector<Event> events_;
  int FindId(std::vector<std::string> ids, std::string id);
  template<typename T>
  int FindId(std::vector<T> list, std::string id, std::function<std::string(T)> func) {
    {
      for (int i =0; i < list.size(); ++i) {
        if (func(list[i]) == id)
          return i;
      }
      throw "ID not found";
    }
  };
  int FindResourceGroup(std::string id) { return FindId<ResourceGroup>(resource_groups_, id, [](auto rg) -> auto { return rg.id;});};
};
}
#endif // SIMPLESATSOLVER_SRC_HSTT_HSTT_H_
