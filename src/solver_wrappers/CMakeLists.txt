add_library(solvers simple_solver.cc simple_solver.h i_solver.cc)
target_link_libraries(solvers sat)
target_link_libraries(solvers solver)
