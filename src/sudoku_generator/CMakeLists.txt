add_library(sudoku_generator generator.cc generator.h generator_v_2.cc generator_v_2.h sudoku_domain.h)

target_link_libraries(sudoku_generator sat sudoku solvers)

add_executable(sudoku_generator_main main.cc)
target_link_libraries(sudoku_generator_main sudoku_generator solver)