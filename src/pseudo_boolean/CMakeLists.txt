
add_executable(pseudo_boolean_test main.cc opb_parser.cc opb_parser.h)
target_link_libraries(pseudo_boolean_test logger pumpkin sat solvers -static)

