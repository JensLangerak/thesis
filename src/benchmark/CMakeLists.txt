add_library(benchmark benchmark.cc benchmark.h globals.h)
target_link_libraries(benchmark logger pumpkin sat )
