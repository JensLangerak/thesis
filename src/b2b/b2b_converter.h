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

  /// Get the index of the var that specifies if meeting meeting
  /// is scheduled at time timeslot.
  /// \param meeting the meeting index.
  /// \param timeslot the timeslot.
  /// \return the index of the meeting timeslot combination.
  inline int GetMeetingTimeslotIndex(int meeting, int timeslot) {
    return timeslot * nr_meetings_ + meeting;
  };

  /// Get the index of the var that specicifies if the person has a meeting at
  /// the given timeslot.
  /// \param person index of the person.
  /// \param timeslot the timeslot.
  /// \return the index of the person timeslot combination.
  inline int GetPersonTimeslotIndex(int person, int timeslot) {
    return timeslot * nr_persons_ + person + nr_meetings_ * nr_timeslots_;
  };

  /// Construct the corresponding sat problem.
  /// \return the sat problem.
  sat::SatProblem ToSat();
  /// Decode the solution found for the sat problem that was returned by ToSat.
  /// The decoded solution is an array that specifies for each meeting the
  /// timeslot that it is scheduled.
  /// The index in the array is the meeting index, and the value is the
  /// timeslot.
  /// \param solution the solution for the sat problem that was returned by
  /// ToSat.
  /// \return An array that specifies for each meeting the timeslot.
  std::vector<int> DecodeSolution(std::vector<bool> solution);

private:
  /// For each meeting add a constraint to the sat problem that specifies if is
  /// should be scheduled in the morning, afternoon, or any of the two.
  void AddTimeOfDayConstraints();
  /// Add constraints to the sat problem that disallow meetings to be scheduled
  /// in the forbidden timeslots of any of its participants.
  void AddForbiddenConstraints();
  /// Add constraints to the sat problem that makes a connection to the
  /// scheduled meeting time vars and the vars that specifies for each person
  /// when he has a meeting.
  void AddMeetingPersonImplications();

  /// Add constraints to the sat problem that forces that each person can have
  /// only one meeting at a specific time.
  void AddOneMeetingPerPersonTime();

  /// Add constraints to the sat problem that specifies the max number of
  /// meetings at the same time.
  void AddMaxMeetings();

  /// Add constraints to the sat problem that specifies that each meeting must
  /// be scheduled exactly once.
  void AddMeetingOnce();

  /// Check if the given solution fulfills all the requirements.
  /// Throws an exception in case of a violation.
  void ValidateSchedule(std::vector<int> vector);

  const B2B problem_;
  sat::SatProblem sat_problem_;

  int nr_meetings_;
  int nr_persons_;
  int nr_timeslots_;
  int nr_locations_;
};
} // namespace simple_sat_solver::b2b

#endif // SIMPLESATSOLVER_SRC_B2B_B2B_CONVERTER_H_
