add_library(sudoku_solver encoder.cc encoder.h)

target_link_libraries(sudoku_solver solver)