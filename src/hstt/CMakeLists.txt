add_library(hstt xhstt_parser.cc xhstt_parser.h hstt.h hstt.cc)
target_link_libraries(hstt sat tinyxml2)


add_executable(hstt_benchmark main.cc)
target_link_libraries(hstt_benchmark hstt)
