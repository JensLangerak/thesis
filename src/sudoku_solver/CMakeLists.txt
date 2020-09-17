add_library(sudoku_solver encoder.cc encoder.h benchmark_parser.cc benchmark_parser.h sudoku_solver.cc sudoku_solver.h)

target_link_libraries(sudoku_solver solver)

add_executable(sudoku_solver_benchmark main.cc)
target_link_libraries(sudoku_solver_benchmark sudoku_solver)