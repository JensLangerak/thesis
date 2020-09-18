//
// Created by jens on 18-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_B2B_B2B_H_
#define SIMPLESATSOLVER_SRC_B2B_B2B_H_
#include <vector>
namespace simple_sat_solver::b2b {
typedef enum class TimeOfDay { kMorning, kAfternoon, kDontCare } TimeOfDay;
struct Meeting {
  int p1;
  int p2;
  TimeOfDay time_of_day;
};
struct B2B {
  int nr_participants;
  int nr_timeslots;
  int nr_locations;
  int nr_morning_slots;
  std::vector<Meeting> meetings;
  std::vector<std::vector<int>> forbidden_slots;
};
} // namespace simple_sat_solver::b2b
#endif // SIMPLESATSOLVER_SRC_B2B_B2B_H_
