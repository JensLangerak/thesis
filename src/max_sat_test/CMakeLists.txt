
add_executable(max_sat main.cc max_sat_bench_mark.cc max_sat_bench_mark.h)
target_link_libraries(max_sat benchmark logger pumpkin -static)

