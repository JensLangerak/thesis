add_library(b2b b2b.h b2b_parser.cc b2b_parser.h)

add_executable(b2b_benchmark main.cc)
target_link_libraries(b2b_benchmark b2b)