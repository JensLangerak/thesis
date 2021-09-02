

add_library(ctt parser.cc parser.h ctt.h ctt_converter.cc ctt_converter.h ctt_benchmark.cc ctt_benchmark.h)
target_link_libraries(ctt sat solvers pumpkin)


add_executable(ctt_benchmark main.cc)
target_link_libraries(ctt_benchmark ctt logger benchmark)

add_executable(ctt_validator validator.cc)
