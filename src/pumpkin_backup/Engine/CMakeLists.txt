
add_library(engine
        constraint_satisfaction_solver.cpp  solver_state.h      variable_selector.cpp
        constraint_optimisation_solver.cpp  constraint_satisfaction_solver.h    value_selector.cpp  variable_selector.h
        constraint_optimisation_solver.h    solver_state.cpp                    value_selector.h
        )

target_link_libraries(engine basic_data_structures propagators pseudo_boolean_encoders logger)