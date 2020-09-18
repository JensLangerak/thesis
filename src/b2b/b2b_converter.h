//
// Created by jens on 18-09-20.
//

#ifndef SIMPLESATSOLVER_SRC_B2B_B2B_CONVERTER_H_
#define SIMPLESATSOLVER_SRC_B2B_B2B_CONVERTER_H_

#include "../sat/sat_problem.h"
#include "b2b.h"
namespace simple_sat_solver::b2b {
class B2bConverter {
public:
  explicit B2bConverter(B2B problem);

  sat::SatProblem ToSat();

private:
  const B2B problem_;
  sat::SatProblem sat_problem_;

  inline int GetMeetingTimeslotIndex(int meeting, int timeslot) {
    return timeslot * nr_meetings_ + meeting;
  };
  inline int GetPersonTimeslotIndex(int person, int timeslot) {
    return timeslot * nr_persons_ + person + nr_meetings_ * nr_timeslots_;
  };
  int nr_meetings_;
  int nr_persons_;
  int nr_timeslots_;
  void AddTimeOfDayConstraints();
  void AddForbiddenConstraints();
  void AddMeetingPersonImplications();

  void AddOneMeetingPerPersonTime();
  void AddMaxMeetings();
};
} // namespace simple_sat_solver::b2b

#endif // SIMPLESATSOLVER_SRC_B2B_B2B_CONVERTER_H_
