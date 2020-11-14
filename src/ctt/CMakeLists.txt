add_library(ctt parser.cc parser.h ctt.h ctt_converter.cc ctt_converter.h)
target_link_libraries(ctt sat solvers)


add_executable(ctt_benchmark main.cc)
target_link_libraries(ctt_benchmark ctt)

add_executable(ctt_validator validator.cc)
