add_library(sudoku_generator generator.cc generator.h)

target_link_libraries(sudoku_generator sat sudoku solvers)

add_executable(sudoku_generator_main main.cc)
target_link_libraries(sudoku_generator_main sudoku_generator solver)