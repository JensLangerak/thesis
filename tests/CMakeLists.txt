enable_testing()
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_SOURCE_DIR} )
#add_subdirectory(lib/catch2)
add_subdirectory(sudoku)
add_subdirectory(sat)
add_subdirectory(b2b)
add_subdirectory(pumpkin)
add_executable(Catch_tests_run main_test.cc)
target_link_libraries (Catch_tests_run
        -Wl,--whole-archive
#        sudoku_test
#        sat_test
#        b2b_test
        pumpkin_test
        -Wl,--no-whole-archive
        )