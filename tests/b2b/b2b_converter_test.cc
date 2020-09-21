//
// Created by jens on 20-09-20.
//

#include "b2b_converter.h"

#include <src/b2b/b2b.h>
#include <string>
#include <vector>

#include "lib/catch2/catch.hpp"
#include "src/sat/sat_problem.h"
#include "src/solver_wrappers/simple_solver.h"

namespace simple_sat_solver::b2b {
using sat::Lit;
using sat::SatProblem;
// TODO move to helper class (is also in the sat_problem_test)
bool TestPartialAssignment(SatProblem s, std::vector<bool> assignments) {
  std::string info_string;
  for (int i = 0; i < assignments.size(); i++) {
    s.AddClause({Lit(i, !assignments[i])});
    info_string += assignments[i] ? "T " : "F ";
  }
  UNSCOPED_INFO(info_string);
  return solver_wrappers::SimpleSolver().Solve(s);
}
TEST_CASE("B2B meeting", "[b2b]") {
  B2B problem;
  problem.nr_locations = 3;
  problem.nr_timeslots = 5;
  problem.nr_participants = 10;
  problem.nr_morning_slots = 2;
  problem.forbidden_slots =
      std::vector<std::vector<int>>(problem.nr_participants);
  SECTION("No meetings") {
    B2bConverter converter(problem);
    REQUIRE(TestPartialAssignment(converter.ToSat(), {}));
  }
  SECTION("Single meeting") {
    problem.meetings.emplace_back(0, 1, TimeOfDay::kDontCare);
    SECTION("Unscheduled meeting") {
      B2bConverter converter(problem);
      SatProblem sat = converter.ToSat();
      for (int i = 0; i < problem.nr_timeslots; i++) {
        sat.AddClause({~Lit(converter.GetMeetingTimeslotIndex(0, i))});
      }
      REQUIRE_FALSE(TestPartialAssignment(sat, {}));
    }
    SECTION("Scheduled meeting") {
      B2bConverter converter(problem);
      SatProblem sat = converter.ToSat();
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(0, 0))});
      REQUIRE(TestPartialAssignment(sat, {}));
    }
    SECTION("Duplicated scheduled meeting") {
      B2bConverter converter(problem);
      SatProblem sat = converter.ToSat();
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(0, 0))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(0, 1))});
      REQUIRE_FALSE(TestPartialAssignment(sat, {}));
    }
  }
  SECTION("Morning meeting") {
    problem.meetings.push_back(Meeting(0, 1, TimeOfDay::kMorning));
    SECTION("Scheduled morning") {
      B2bConverter converter(problem);
      auto t = GENERATE(0, 1);
      SatProblem sat = converter.ToSat();
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(0, t))});
      INFO(t);
      REQUIRE(TestPartialAssignment(sat, {}));
    }
    SECTION("Scheduled afternoon") {
      B2bConverter converter(problem);
      auto t = GENERATE(2, 3, 4);
      SatProblem sat = converter.ToSat();
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(0, t))});
      INFO(t);
      REQUIRE_FALSE(TestPartialAssignment(sat, {}));
    }
  }
  SECTION("Afternoon meeting") {
    problem.meetings.push_back(Meeting(0, 1, TimeOfDay::kAfternoon));
    SECTION("Scheduled morning") {
      B2bConverter converter(problem);
      auto t = GENERATE(0, 1);
      SatProblem sat = converter.ToSat();
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(0, t))});
      INFO(t);
      REQUIRE_FALSE(TestPartialAssignment(sat, {}));
    }
    SECTION("Scheduled afternoon") {
      B2bConverter converter(problem);
      auto t = GENERATE(2, 3, 4);
      SatProblem sat = converter.ToSat();
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(0, t))});
      INFO(t);
      REQUIRE(TestPartialAssignment(sat, {}));
    }
  }
  SECTION("Dont care meeting") {
    problem.meetings.push_back(Meeting(0, 1, TimeOfDay::kDontCare));
    SECTION("Scheduled morning") {
      B2bConverter converter(problem);
      auto t = GENERATE(0, 1);
      SatProblem sat = converter.ToSat();
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(0, t))});
      INFO(t);
      REQUIRE(TestPartialAssignment(sat, {}));
    }
    SECTION("Scheduled afternoon") {
      B2bConverter converter(problem);
      auto t = GENERATE(2, 3, 4);
      SatProblem sat = converter.ToSat();
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(0, t))});
      INFO(t);
      REQUIRE(TestPartialAssignment(sat, {}));
    }
  }
  SECTION("Person forbidden slot") {
    problem.forbidden_slots[2].push_back(4);
    problem.forbidden_slots[2].push_back(2);
    SECTION("Scheduled in forbidden p1") {
      problem.meetings.emplace_back(2, 0,TimeOfDay::kDontCare);

      B2bConverter converter(problem);
      SatProblem sat = converter.ToSat();
      SECTION("Illegal times") {
        auto t = GENERATE(2, 4);
        sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(0, t))});
        INFO(t);
        REQUIRE_FALSE(TestPartialAssignment(sat, {}));
      }
      SECTION("Legal times") {
        auto t = GENERATE(0,1,3);
        sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(0, t))});
        INFO(t);
        REQUIRE(TestPartialAssignment(sat, {}));
      }
    }
    SECTION("Scheduled in forbidden p2") {
      problem.meetings.emplace_back(3, 2,TimeOfDay::kDontCare);

      B2bConverter converter(problem);
      SatProblem sat = converter.ToSat();
      SECTION("Illegal times") {
        auto t = GENERATE(2, 4);
        sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(0, t))});
        INFO(t);
        REQUIRE_FALSE(TestPartialAssignment(sat, {}));
      }
      SECTION("Legal times") {
        auto t = GENERATE(0,1,3);
        sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(0, t))});
        INFO(t);
        REQUIRE(TestPartialAssignment(sat, {}));
      }
    }
  }
  SECTION("Person multiple meetings") {
    problem.meetings.emplace_back(0,1,TimeOfDay::kDontCare);
    problem.meetings.emplace_back(3,0, TimeOfDay ::kDontCare);
    SECTION("Same time") {
      B2bConverter converter(problem);
      SatProblem sat = converter.ToSat();
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(0, 0))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(1, 0))});
      REQUIRE_FALSE(TestPartialAssignment(sat, {}));
    }
    SECTION("Different time") {

      B2bConverter converter(problem);
      SatProblem sat = converter.ToSat();
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(0, 1))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(1, 0))});
      REQUIRE(TestPartialAssignment(sat, {}));
    }
  }
  SECTION("Nr meetings per slot") {
    problem.meetings.emplace_back(0,1,TimeOfDay::kDontCare);
    problem.meetings.emplace_back(2,3,TimeOfDay::kDontCare);
    problem.meetings.emplace_back(4,5,TimeOfDay::kDontCare);
    problem.meetings.emplace_back(6,7,TimeOfDay::kDontCare);
    problem.meetings.emplace_back(8,9,TimeOfDay::kDontCare);
    problem.meetings.emplace_back(0,1,TimeOfDay::kDontCare);
    problem.meetings.emplace_back(2,3,TimeOfDay::kDontCare);
    problem.meetings.emplace_back(4,5,TimeOfDay::kDontCare);
    problem.meetings.emplace_back(6,7,TimeOfDay::kDontCare);

    problem.meetings.emplace_back(8,9,TimeOfDay::kDontCare);
    B2bConverter converter(problem);
    SatProblem sat = converter.ToSat();
    SECTION("Max nr_locations") {
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(0, 0))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(1, 0))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(2, 0))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(3, 1))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(4, 1))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(5, 1))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(6, 2))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(7, 2))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(8, 2))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(9, 3))});
      REQUIRE(TestPartialAssignment(sat, {}));
    }
    SECTION("One to many scheduled") {
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(0, 0))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(1, 0))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(2, 1))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(3, 1))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(4, 1))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(5, 1))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(6, 2))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(7, 2))});

      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(8, 3))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(9, 3))});
      REQUIRE_FALSE(TestPartialAssignment(sat, {}));
    }
    SECTION("Way to many scheduled") {
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(0, 0))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(1, 0))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(2, 0))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(3, 0))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(4, 0))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(5, 1))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(6, 2))});
      sat.AddClause({Lit(converter.GetMeetingTimeslotIndex(7, 2))});
      REQUIRE_FALSE(TestPartialAssignment(sat, {}));
    }
  }
}
} // namespace simple_sat_solver::b2b