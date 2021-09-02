add_library(pumpkin_test STATIC propagators/pb_sum_test.cc )
target_link_libraries(pumpkin_test  pumpkin sat solvers)
#target_include_directories(sat_test PRIVATE ${PROJECT_SOURCE_DIR}/src/sat)
