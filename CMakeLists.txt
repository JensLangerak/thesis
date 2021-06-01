cmake_minimum_required(VERSION 3.17)
project(SimpleSatSolver)

set(CMAKE_CXX_STANDARD 17)

#SET(CMAKE_C_COMPILER   /home/jens/build/gcc49/pkg/gcc49/usr/bin/gcc-4.9)
#SET(CMAKE_CXX_COMPILER   /home/jens/build/gcc49/pkg/gcc49/usr/bin/g++-4.9)


add_subdirectory(lib)
add_subdirectory(src)
#add_subdirectory(tests)
#add_executable(SimpleSatSolver main.cc src/solver/solver.cc src/solver/solver.h src/solver/types.h src/solver/var_order.cc src/solver/var_order.h src/solver/clause.cc src/solver/clause.h src/solver/parser.cc src/solver/parser.h src/sudoku/encoder.cc src/sudoku/encoder.h src/sudoku/sat_problem.h)
add_executable(SimpleSatSolver main.cc)
target_link_libraries(SimpleSatSolver solver)