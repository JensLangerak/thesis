//
// Created by jens on 11-11-20.
//

#ifndef SIMPLESATSOLVER_SRC_CTT_CTT_H_
#define SIMPLESATSOLVER_SRC_CTT_CTT_H_

#include <string>
#include <vector>
namespace simple_sat_solver::ctt {
struct Data {
  int index;
};

struct Item : Data {
  std::string id;
};

struct Teacher : Item {
};
struct Course : Item {
  int teacher_index;
  int nr_lectures;
  int min_working_days;
  int nr_students;

};

struct Room : Item {
  int max_capactity;
};

struct Curriculum : Item {
  int nr_courses;
  std::vector<int> courses_indices;
};

struct UnavailabilityConstraint : Data {
  int course_index;
  int day;
  int period;
};


struct Ctt {
  std::string name;
  int nr_courses;
  int nr_rooms;
  int nr_days;
  int nr_periods_per_day;
  int nr_curricula;
  int nr_constraints;

  std::vector<Teacher> teachers;
  std::vector<Course> courses;
  std::vector<Room> rooms;
  std::vector<Curriculum> curricula;
  std::vector<UnavailabilityConstraint> constraints;

  template<typename T>
  int FindId(std::vector<T> list, std::string id) {
    static_assert(std::is_base_of<Item, T>::value, "T must derive from Item");
      for (int i = 0; i < list.size(); ++i) {
      Item * item = dynamic_cast<Item *>(&list[i]);
      if (item->id == id)
        return i;
    }
    return -1;
  }

};

struct CttAssignment {
  CttAssignment() {};
  CttAssignment(int course_id, int room_id, int day, int period) : course_index(course_id), room_index(room_id), day(day), period(period) {};
  int course_index;
  int room_index;
  int day;
  int period;
};


}

#endif // SIMPLESATSOLVER_SRC_CTT_CTT_H_
