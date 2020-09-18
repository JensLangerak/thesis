//
// Created by jens on 18-09-20.
//

#include "b2b_converter.h"
namespace simple_sat_solver::b2b {

using sat::Lit;
B2bConverter::B2bConverter(B2B problem) : problem_(problem), sat_problem_(0) {}
sat::SatProblem B2bConverter::ToSat() {
  nr_meetings_ = problem_.meetings.size();
  nr_persons_ = problem_.nr_participants;
  nr_timeslots_ = problem_.nr_timeslots;
  sat_problem_ = sat::SatProblem(nr_meetings_ * nr_timeslots_ +
                                 nr_persons_ * nr_timeslots_);
  AddTimeOfDayConstraints();
  AddForbiddenConstraints();
  AddMeetingPersonImplications();
  AddOneMeetingPerPersonTime();
  AddMaxMeetings();
  return sat_problem_;
}

void B2bConverter::AddTimeOfDayConstraints() {
  for (int i = 0; i < problem_.meetings.size(); i++) {
    switch (problem_.meetings[i].time_of_day) {
    case TimeOfDay::kMorning:
      for (int t = problem_.nr_morning_slots; t < problem_.nr_timeslots; t++)
        sat_problem_.AddClause({Lit(GetMeetingTimeslotIndex(i, t), true)});
      break;
    case TimeOfDay::kAfternoon:
      for (int t = 0; t < problem_.nr_morning_slots; t++)
        sat_problem_.AddClause({Lit(GetMeetingTimeslotIndex(i, t), true)});
      break;
    case TimeOfDay::kDontCare:
      break;
    }
  }
}
void B2bConverter::AddForbiddenConstraints() {
  for (int i = 0; i < problem_.nr_participants; i++) {
    for (int t : problem_.forbidden_slots[i]) {
      sat_problem_.AddClause({Lit(GetPersonTimeslotIndex(i, t), true)});
    }
  }
}
void B2bConverter::AddMeetingPersonImplications() {
  for (int i = 0; i < nr_meetings_; i++) {
    Meeting m = problem_.meetings[i];
    for (int t = 0; t < nr_timeslots_; t++) {
      sat_problem_.Implies(Lit(GetMeetingTimeslotIndex(i, t)),
                           Lit(GetPersonTimeslotIndex(m.p1, t)));
      sat_problem_.Implies(Lit(GetMeetingTimeslotIndex(i, t)),
                           Lit(GetPersonTimeslotIndex(m.p2, t)));
    }
  }
}
void B2bConverter::AddOneMeetingPerPersonTime() {
  for (int p = 0; p < nr_persons_; p++) {
    std::vector<int> meetings;
    for (int i = 0; i < nr_meetings_; i++) {
      Meeting m = problem_.meetings[i];
      if (m.p1 == p || m.p2 == p)
        meetings.push_back(i);
    }
    for (int t = 0; t < nr_timeslots_; t++) {
      std::vector<Lit> at_most_one;
      at_most_one.reserve(meetings.size());
      for (int m : meetings) {
        at_most_one.push_back(Lit(GetMeetingTimeslotIndex(m, t)));
      }
      sat_problem_.AtMostOne(at_most_one);
    }
  }
}
void B2bConverter::AddMaxMeetings() {
  for (int t = 0; t < nr_timeslots_; t++) {
    std::vector<Lit> at_most_k;
    for (int m = 0; m < nr_meetings_; m++) {
      at_most_k.push_back(Lit(GetMeetingTimeslotIndex(m, t)));
    }
    sat_problem_.AtMostK(nr_timeslots_, at_most_k);
  }
}
} // namespace simple_sat_solver::b2b