cmake_minimum_required(VERSION 3.17)
project(SimpleSatSolver)

set(CMAKE_CXX_STANDARD 17)

add_executable(SimpleSatSolver main.cc src/solver.cc src/solver.h src/types.h src/var_order.cc src/var_order.h src/constr.cc src/constr.h src/clause.cc src/clause.h)