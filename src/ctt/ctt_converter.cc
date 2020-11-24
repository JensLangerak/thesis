//
// Created by jens on 12-11-20.
//

#include "ctt_converter.h"
#include "../sat/constraints/cardinality_constraint.h"
#include "../sat/constraints/sum_constraint.h"
#include "../sat/encoders/totaliser_encoder.h"
#include <cassert>
#include <fstream>
namespace simple_sat_solver::ctt {

CttConverter::CttConverter(Ctt problem)
    : ctt_problem_(problem), sat_problem_(0),
      total_timeslots_(problem.nr_periods_per_day * problem.nr_days) {
  sat_problem_.AddNewVars(ctt_problem_.nr_courses * ctt_problem_.nr_rooms *
                          total_timeslots_);
  course_schedule_start_index =
      sat_problem_.AddNewVars(ctt_problem_.nr_courses * total_timeslots_);
  course_schedule_end_index = sat_problem_.GetNrVars();
  course_days_start_index =
      sat_problem_.AddNewVars(ctt_problem_.nr_courses * ctt_problem_.nr_days);
  curriculum_time_start_index = sat_problem_.AddNewVars(ctt_problem_.nr_curricula * total_timeslots_);
  course_room_start_index = sat_problem_.AddNewVars(ctt_problem_.nr_rooms * ctt_problem_.nr_courses);
  AddScheduleLecturesConstraints();
  AddRoomOccupancy();
  CurriculumConflicts();
  TeacherConflicts();
  AddUnavailabiltyConstraints();
  AddMinWorkingDayConstraints();
  AddCompactnessConstraints();
  AddRoomCapacityConstraints();
  AddRoomStability();
  AddPenalty();
}
int CttConverter::CourseRoomScheduleIndex(int course_index, int room_index,
                                          int time_index) {
  assert(course_index < ctt_problem_.nr_courses);
 assert(room_index < ctt_problem_.nr_rooms);
 assert(time_index < total_timeslots_);

  return ((course_index * ctt_problem_.nr_rooms) + room_index) *
             total_timeslots_ +
         time_index;
}
int CttConverter::TimeIndex(int day, int period) {
  assert(day < ctt_problem_.nr_days);
  assert(period < ctt_problem_.nr_periods_per_day);

  return day * ctt_problem_.nr_periods_per_day + period;
}
void CttConverter::AddScheduleLecturesConstraints() {
  for (Course c : ctt_problem_.courses) {
    ScheduleOncePerTime(c);
    ProjectOnCourseTimeVars(c);
    ProjectOnCourseDayVars(c);
    ScheduleAllLectures(c);
  }
}
void CttConverter::ScheduleOncePerTime(Course course) {
  for (int t = 0; t < total_timeslots_; ++t) {
    std::vector<Lit> literals_for_c_at_t;
    for (Room r : ctt_problem_.rooms) {
      literals_for_c_at_t.push_back(
          Lit(CourseRoomScheduleIndex(course.index, r.index, t)));
    }
    sat_problem_.AtMostOne(literals_for_c_at_t);
  }
}
void CttConverter::ScheduleAllLectures(Course course) {
  std::vector<Lit> literals_for_c;
  std::vector<Lit> false_lits;
  for (int t = 0; t < total_timeslots_; ++t) {
    literals_for_c.push_back(Lit(CourseScheduleIndex(course.index, t)));
    false_lits.push_back(~Lit(CourseScheduleIndex(course.index, t)));
  }
  std::cout << course.index << " - " << literals_for_c[0].x << "  -  "
            << course.nr_lectures << std::endl;

  // TODO fix bugs
  //    sat::TotaliserEncoder::Encode(sat_problem_, literals_for_c,
  //                                  course.nr_lectures, course.nr_lectures);

  //  sat_problem_.AddConstraint(new CardinalityConstraint(literals_for_c, course.nr_lectures, course.nr_lectures));
  if (course.nr_lectures == 1) {
    sat_problem_.ExactlyOne(literals_for_c);
  } else {
    assert(course.nr_lectures > 1);
    sat_problem_.AddConstraint(
        new CardinalityConstraint(literals_for_c, 0, course.nr_lectures));
    assert(false_lits.size() - course.nr_lectures > 1);
    sat_problem_.AddConstraint(new CardinalityConstraint(
        false_lits, 0, false_lits.size() - course.nr_lectures));
  }
}
void CttConverter::AddRoomOccupancy() {
  for (Room r : ctt_problem_.rooms) {
    for (int t = 0; t < total_timeslots_; ++t) {
      std::vector<Lit> literals;
      for (Course c : ctt_problem_.courses) {
        literals.push_back(Lit(CourseRoomScheduleIndex(c.index, r.index, t)));
      }
      sat_problem_.AtMostOne(literals);
    }
  }
}
void CttConverter::CurriculumConflicts() {
  for (Curriculum c : ctt_problem_.curricula) {
    for (int t = 0; t < total_timeslots_; ++t) {
      std::vector<Lit> literals;
      for (int course : c.courses_indices) {
        literals.push_back(CourseScheduleIndex(course, t));
      }
      sat_problem_.AtMostOne(literals);
    }
  }
}

int CttConverter::CourseDayIndex(int course_index, int day_index) {
  assert(course_index < ctt_problem_.nr_courses);
  assert(day_index < ctt_problem_.nr_days);
  return course_days_start_index + course_index * ctt_problem_.nr_days +
         day_index;
}

int CttConverter::CourseRoomIndex(int course_index, int room_index){
  assert(course_index < ctt_problem_.nr_courses);
  assert(room_index < ctt_problem_.nr_rooms);
  return course_room_start_index + course_index * ctt_problem_.nr_rooms +
     room_index;
}
int CttConverter::CourseScheduleIndex(int course_index, int time_index) {
  assert(course_index < ctt_problem_.nr_courses);
  assert(time_index < total_timeslots_);
  return course_schedule_start_index + course_index * total_timeslots_ +
      time_index;
}
int CttConverter::CurriculumTimeIndex(int curriculum_index, int time_index) {
  assert(curriculum_index < ctt_problem_.nr_curricula);
  assert(time_index < total_timeslots_);
  return curriculum_time_start_index + curriculum_index * total_timeslots_ +
         time_index;
}
void CttConverter::ProjectOnCourseTimeVars(Course course) {
  for (int t = 0; t < total_timeslots_; ++t) {
    std::vector<Lit> literals;
    Lit l_d = Lit(CourseScheduleIndex(course.index, t));
    for (Room r : ctt_problem_.rooms) {
      Lit l_r = Lit(CourseRoomScheduleIndex(course.index, r.index, t));
      literals.push_back(l_r);
      sat_problem_.AddClause({~l_r, l_d});
    }
    literals.push_back(~l_d);
    sat_problem_.AddClause(literals);
  }
}
void CttConverter::TeacherConflicts() {
  for (Teacher teacher : ctt_problem_.teachers) {
    for (int t = 0; t < total_timeslots_; ++t) {
      std::vector<Lit> literals;
      for (Course c : ctt_problem_.courses) {
        if (c.teacher_index == teacher.index)
          literals.push_back(CourseScheduleIndex(c.index, t));
      }
      sat_problem_.AtMostOne(literals);
    }
  }
}
void CttConverter::AddUnavailabiltyConstraints() {
  for (UnavailabilityConstraint c : ctt_problem_.constraints) {
    sat_problem_.AddClause({~Lit(
        CourseScheduleIndex(c.course_index, TimeIndex(c.day, c.period)))});
  }
}
std::vector<CttAssignment>
CttConverter::ConvertSolution(std::vector<bool> sat_solution) {
  assert(sat_solution.size() >= sat_problem_.GetNrVars());
  std::vector<CttAssignment> solution;
  for (Course c : ctt_problem_.courses) {
    for (int d = 0; d < ctt_problem_.nr_days; ++d) {
      for (int t = 0; t < ctt_problem_.nr_periods_per_day; ++t) {
        for (Room r : ctt_problem_.rooms) {
          if (sat_solution[CourseRoomScheduleIndex(c.index, r.index,
                                                   TimeIndex(d, t))])
            solution.push_back(CttAssignment(c.index, r.index, d, t));
        }
      }
    }
  }
  int scheduled_count = 0;
  for (int i = 0; i < course_schedule_start_index; ++i) {
    if (sat_solution[i])
      scheduled_count++;
  }
  int scheduled_count2 = 0;
  for (int i = course_schedule_start_index; i < course_schedule_end_index;
       ++i) {
    if (sat_solution[i])
      scheduled_count2++;
  }
  assert(scheduled_count == scheduled_count2);
  assert(scheduled_count == solution.size());

  for (Course c : ctt_problem_.courses) {
    int count = 0;
    for (int t = 0; t < total_timeslots_; ++t) {
      if (sat_solution[CourseScheduleIndex(c.index, t)])
        count++;
    }
    assert(count == c.nr_lectures);
  }

  int score = ValidateSolution(solution);
  return solution;
}
int CttConverter::ValidateSolution(std::vector<CttAssignment> solution) {
  std::vector<std::vector<bool>> course_time(
      ctt_problem_.nr_courses, std::vector<bool>(total_timeslots_, false));
  std::vector<std::vector<bool>> room_time(
      ctt_problem_.nr_rooms, std::vector<bool>(total_timeslots_, false));
  std::vector<std::vector<bool>> teacher_time(
      ctt_problem_.teachers.size(), std::vector<bool>(total_timeslots_, false));
  std::vector<std::vector<bool>> curriculum_time(
      ctt_problem_.nr_curricula, std::vector<bool>(total_timeslots_, false));
  std::vector<std::vector<bool>> course_room(
      ctt_problem_.nr_courses, std::vector<bool>(ctt_problem_.nr_rooms, false));
  for (CttAssignment assignment : solution) {
    int time = TimeIndex(assignment.day, assignment.period);
    Schedule(course_time, assignment.course_index, time,
             "Two lectures at the same time");
    Schedule(room_time, assignment.room_index, time, "Room occupied");
    Schedule(teacher_time,
             ctt_problem_.courses[assignment.course_index].teacher_index, time,
             "Teacher scheduled twice");
    course_room[assignment.course_index][assignment.room_index] = true;
  }
  for (Curriculum c : ctt_problem_.curricula) {
    for (int course : c.courses_indices) {
      for (int t = 0; t < total_timeslots_; ++t) {
        if (course_time[course][t])
          Schedule(curriculum_time, c.index, t, "Curriculum conflict");
      }
    }
  }

  for (Course c : ctt_problem_.courses) {
    int sum = 0;
    for (bool b : course_time[c.index]) {
      if (b)
        ++sum;
    }
    if (sum != c.nr_lectures)
      throw "Not all lectures scheduled";
  }

  for (UnavailabilityConstraint c : ctt_problem_.constraints) {
    if (course_time[c.course_index][TimeIndex(c.day, c.period)])
      throw "Course scheduled at unavailable time";
  }

  int penalty = 0;
  for (CttAssignment a : solution) {
    int nr_students = ctt_problem_.courses[a.course_index].nr_students;
    int max_in_room = ctt_problem_.rooms[a.room_index].max_capactity;
    int score = (nr_students > max_in_room) ? nr_students - max_in_room : 0;
    penalty += score;
  }

  for (Course c : ctt_problem_.courses) {
    int work_days = 0;
    for (int d = 0; d < ctt_problem_.nr_days; ++d) {
      for (int t = 0; t < ctt_problem_.nr_periods_per_day; ++t) {
        if (course_time[c.index][TimeIndex(d, t)]) {
          ++work_days;
          break;
        }
      }
    }
    if (work_days < c.min_working_days)
      penalty += 1 * (c.min_working_days - work_days); //5
  }

  int isolated_lectures = 0;
  for (Curriculum c : ctt_problem_.curricula) {
    for (int d = 0; d < ctt_problem_.nr_days; ++d) {
      for (int t = 0; t < ctt_problem_.nr_periods_per_day; ++t) {
        int time = TimeIndex(d, t);
        if (curriculum_time[c.index][time]) {
          if (t > 0) {
            if (curriculum_time[c.index][TimeIndex(d, t - 1)])
              continue;
          }
          if (t < ctt_problem_.nr_periods_per_day - 1)
            if (curriculum_time[c.index][TimeIndex(d, t + 1)])
              continue;
          ++isolated_lectures;
        }
      }
    }
  }

  penalty += 1 * isolated_lectures; //2

  int room_stability = 0;
  for (Course c : ctt_problem_.courses) {
    int rooms = 0;
    for (Room r : ctt_problem_.rooms) {
      if (course_room[c.index][r.index])
        ++rooms;
    }
    if (rooms > 1)
      room_stability += rooms - 1;
  }
  penalty += room_stability;
  return penalty;
}

void CttConverter::PrintSolution(const std::vector<CttAssignment> &solution,
                                 std::ostream &output) {
  for (const CttAssignment &assignment : solution) {
    output << ctt_problem_.courses[assignment.course_index].id << " "
           << ctt_problem_.rooms[assignment.room_index].id << " "
           << assignment.day << " " << assignment.period << std::endl;
  }
}
void CttConverter::Schedule(std::vector<std::vector<bool>> &schedule, int id,
                            int time, std::string error) {
  if (schedule[id][time])
    throw error;
  schedule[id][time] = true;
}
void CttConverter::ProjectOnCourseDayVars(Course course) {
  for (int d = 0; d < ctt_problem_.nr_days; ++d) {
    std::vector<Lit> literals;
    Lit l_d = Lit(CourseDayIndex(course.index, d));
    for (int t =0; t < ctt_problem_.nr_periods_per_day; ++ t) {
      Lit l_t = Lit(CourseScheduleIndex(course.index, TimeIndex(d, t)));
      literals.push_back(l_t);
      sat_problem_.AddClause({~l_t, l_d});
    }
    literals.push_back(~l_d);
    sat_problem_.AddClause(literals);//TODO not needed?
  }
}
void CttConverter::AddMinWorkingDayConstraints() {
  for (auto c : ctt_problem_.courses) {
    if (c.min_working_days <= 1)
      continue;
//    std::vector<Lit> sum;
//    for (int i = 0; i < c.min_working_days; ++i) {
//      sum.push_back(sat_problem_.AddNewVar());
//    }
    std::vector<Lit> days;
    for (int d = 0; d < ctt_problem_.nr_days; ++d) {
      days.push_back(CourseDayIndex(c.index, d));
    }
    for (int i = 0; i < c.min_working_days - 1; ++i) {
      Lit p = sat_problem_.AddNewVar();
//      for (int i =0; i < 5; ++i)
      penalties_.push_back(p);
      days.push_back(p);
    }
    std::vector<Lit> c_days;
    for (Lit l : days)
      c_days.push_back(~l);
    sat_problem_.AddConstraint(new CardinalityConstraint(c_days,0, c_days.size() - c.min_working_days));
//    sat_problem_.AddConstraint(new SumConstraint(days, sum));

//    for (Lit l : days) {
//      // TODO weight
//      Lit penalty = sat_problem_.AddNewVar();
//      penalties_.push_back(penalty);
//      sat_problem_.AddClause({l, penalty});
//    }
  }
}
void CttConverter::AddCompactnessConstraints() {
  for (Curriculum c : ctt_problem_.curricula) {
    for (int t = 0; t< total_timeslots_; ++t) {
      Lit curriculum_time = CurriculumTimeIndex(c.index, t);
      std::vector<Lit> course_slots;
      for (int course : c.courses_indices) {
        Lit course_time = CourseScheduleIndex(course, t);
        sat_problem_.AddClause({curriculum_time, ~course_time});
        course_slots.push_back(course_time);
      }
      course_slots.push_back(~curriculum_time);
      sat_problem_.AddClause(course_slots);
    }
  }

  for (Curriculum c : ctt_problem_.curricula) {
    for (int d = 0; d < ctt_problem_.nr_days; ++d) {
      for (int p = 0; p < ctt_problem_.nr_periods_per_day; ++p) {
        int t = TimeIndex(d, p);
        std::vector<Lit> clause;
        Lit penalty = sat_problem_.AddNewVar();
//        for (int i =0; i < 2; ++i)
        penalties_.push_back(penalty);
        clause.push_back(penalty);
        clause.push_back(~Lit(CurriculumTimeIndex(c.index, t)));
        if (p > 0)
          clause.push_back(Lit(CurriculumTimeIndex(c.index, t - 1)));
        if (p < ctt_problem_.nr_periods_per_day - 1)
          clause.push_back(Lit(CurriculumTimeIndex(c.index, t + 1)));
        sat_problem_.AddClause(clause);
      }
    }
  }

}
void CttConverter::AddRoomCapacityConstraints() {
  for (Course c : ctt_problem_.courses) {
    for (Room r : ctt_problem_.rooms) {
      if (c.nr_students <= r.max_capactity)
        continue;
      for (int t = 0; t < total_timeslots_; ++t) {
        if (c.nr_students > r.max_capactity) {
          Lit penalty = sat_problem_.AddNewVar();
          sat_problem_.AddClause(
              {penalty, ~Lit(CourseRoomScheduleIndex(c.index, r.index, t))});
//          for (int i = 0; i < c.nr_students - r.max_capactity; ++i)
            // TODO weights
            penalties_.push_back(penalty);
        }
      }

    }
  }
}
void CttConverter::AddRoomStability() {
  for (Course c : ctt_problem_.courses) {
    for (Room r : ctt_problem_.rooms) {
      std::vector<Lit> rooms;
      Lit course_room = CourseRoomIndex(c.index, r.index);
      for (int t = 0; t<total_timeslots_;++t) {
        Lit r_l = CourseRoomScheduleIndex(c.index, r.index, t);
        sat_problem_.AddClause({~r_l, course_room});
        rooms.push_back(r_l);
      }
      rooms.push_back(~course_room);
      sat_problem_.AddClause(rooms);
    }
  }

  for (Course c : ctt_problem_.courses) {
    std::vector<Lit> input;
//    std::vector<Lit> sum;
    for (Room r : ctt_problem_.rooms) {
//      sum.push_back(sat_problem_.AddNewVar());
      input.push_back(CourseRoomIndex(c.index, r.index));
    }
    for (int i = 0; i < ctt_problem_.nr_rooms; ++i) {
      Lit l = sat_problem_.AddNewVar();
      penalties_.push_back(l);
      input.push_back(~l);
    }
    sat_problem_.AddConstraint(new CardinalityConstraint(input, 0, ctt_problem_.nr_rooms + 1));


//    sat_problem_.AddConstraint(new SumConstraint(input, sum));
//    for (int i = 1; i < sum.size(); ++i) {
//      penalties_.push_back(sum[i]);
//    }
  }
}
void CttConverter::AddPenalty() {
  int test = penalties_.size();
  for (Lit l : penalties_)
    sat_problem_.AddToMinimize(l);
}
} // namespace simple_sat_solver::ctt
