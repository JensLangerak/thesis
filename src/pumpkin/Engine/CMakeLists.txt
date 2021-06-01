
add_library(engine
        conflict_analysis_result_clausal.cpp conflict_analysis_result_clausal.h
        constraint_satisfaction_solver.cpp  solver_state.h      variable_selector.cpp
        constraint_optimisation_solver.cpp  constraint_satisfaction_solver.h    value_selector.cpp  variable_selector.h
        constraint_optimisation_solver.h    solver_state.cpp                    value_selector.h
        learned_clause_minimiser.cpp
        learned_clause_minimiser.h
        lower_bound_search.cpp
        lower_bound_search.h
        preprocessor.cpp
        preprocessor.h
        upper_bound_search.cpp
        upper_bound_search.h
        )

target_link_libraries(engine utilities propagators pseudo_boolean_encoders)