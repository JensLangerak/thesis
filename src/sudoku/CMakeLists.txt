add_library(sudoku encoder.cc encoder.h benchmark_parser.cc benchmark_parser.h)

target_link_libraries(sudoku solver)
target_link_libraries(sudoku sat)

add_executable(sudoku_solver_benchmark main.cc)
target_link_libraries(sudoku_solver_benchmark sudoku)
target_link_libraries(sudoku_solver_benchmark solvers)