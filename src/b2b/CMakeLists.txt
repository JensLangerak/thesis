add_library(b2b b2b.h b2b_parser.cc b2b_parser.h b2b_converter.cc b2b_converter.h)
target_link_libraries(b2b sat solvers)


add_executable(b2b_benchmark main.cc)
target_link_libraries(b2b_benchmark b2b)