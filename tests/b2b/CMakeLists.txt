add_library(b2b_test STATIC b2b_converter_test.cc)
target_link_libraries(b2b_test sat solvers b2b)
target_include_directories(b2b_test PRIVATE ${PROJECT_SOURCE_DIR}/src/b2b)
