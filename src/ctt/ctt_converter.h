//
// Created by jens on 12-11-20.
//

#ifndef SIMPLESATSOLVER_SRC_CTT_CTT_CONVERTER_H_
#define SIMPLESATSOLVER_SRC_CTT_CTT_CONVERTER_H_

#include "../sat/encoders/totaliser_encoder.h"
#include "../sat/sat_problem.h"
#include "ctt.h"
#include <iostream>
using namespace simple_sat_solver::sat;
namespace simple_sat_solver::ctt {
class CttConverter {
public:
  explicit CttConverter(Ctt problem);
  SatProblem *GetSatProblem() { return &sat_problem_;};
  std::vector<CttAssignment> ConvertSolution(std::vector<bool> sat_solution);

  int ValidateSolution(std::vector<CttAssignment> solution);

  void PrintSolution(const std::vector<CttAssignment> &solution, std::ostream &output);
  void PrintSolution(const std::vector<CttAssignment> &solution) {PrintSolution(solution, std::cout);};
private:
  const Ctt ctt_problem_;
  SatProblem sat_problem_;

  int CourseRoomScheduleIndex(int course_index, int room_index, int time_index);
  int CourseRoomIndex(int course_index, int room_index);
  int CourseScheduleIndex(int course_index, int time_index);
  int CourseDayIndex(int course_index, int day_index);
  int CurriculumTimeIndex(int curriculum_index, int time_index);
  int course_schedule_start_index;
  int TimeIndex(int day, int period);
  const int total_timeslots_;

  void AddScheduleLecturesConstraints();
  void ScheduleOncePerTime(Course course);
  void ScheduleAllLectures(Course course);
  void AddRoomOccupancy();
  void CurriculumConflicts();
  void ProjectOnCourseTimeVars(Course course);
  void TeacherConflicts();
  void AddUnavailabiltyConstraints();
  void Schedule(std::vector<std::vector<bool>> &schedule, int id, int time,
                std::string error);
  int course_schedule_end_index;
  int course_days_start_index;
  int curriculum_time_start_index;
  int course_room_start_index;
  void ProjectOnCourseDayVars(Course course);

  std::vector<WeightedLit> penalties_;
  void AddMinWorkingDayConstraints();
  void AddCompactnessConstraints();
  void AddRoomCapacityConstraints();
  void AddRoomStability();
  void AddPenalty();
  int course_room_schedule_start_index;
};
}
#endif // SIMPLESATSOLVER_SRC_CTT_CTT_CONVERTER_H_
