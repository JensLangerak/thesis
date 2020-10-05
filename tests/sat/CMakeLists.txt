add_library(sat_test STATIC types_test.cc sat_problem_test.cc encoders/totaliser_encoder_test.cc)
target_link_libraries(sat_test sat solvers)
target_include_directories(sat_test PRIVATE ${PROJECT_SOURCE_DIR}/src/sat)
