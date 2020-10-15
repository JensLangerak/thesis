

find_library(cryptominisat NAMES libcryptominisat5 PATH /usr/local/lib)

add_library(solvers simple_solver.cc simple_solver.h i_solver.cc crypto_mini_sat.cc crypto_mini_sat.h pumpkin.cc pumpkin.h)
target_link_libraries(solvers sat solver ${cryptominisat} pumpkin)
