add_library(sudoku_test STATIC encoder_test.cc)
target_link_libraries(sudoku_test sudoku)
target_include_directories(sudoku_test PRIVATE ${PROJECT_SOURCE_DIR}/src/sudoku)
