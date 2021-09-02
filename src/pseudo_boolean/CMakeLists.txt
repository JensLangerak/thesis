
add_executable(pseudo_boolean_test main.cc opb_parser.cc opb_parser.h opb_benchmark.cc opb_benchmark.h)
target_link_libraries(pseudo_boolean_test benchmark logger pumpkin -static)

