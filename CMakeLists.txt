cmake_minimum_required(VERSION 3.17)
project(SimpleSatSolver)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)
add_subdirectory(tests)
#add_executable(SimpleSatSolver main.cc src/solver/solver.cc src/solver/solver.h src/solver/types.h src/solver/var_order.cc src/solver/var_order.h src/solver/clause.cc src/solver/clause.h src/solver/parser.cc src/solver/parser.h src/sudoku_solver/encoder.cc src/sudoku_solver/encoder.h src/sudoku_solver/sat_problem.h)
add_executable(SimpleSatSolver main.cc)
target_link_libraries(SimpleSatSolver solver)