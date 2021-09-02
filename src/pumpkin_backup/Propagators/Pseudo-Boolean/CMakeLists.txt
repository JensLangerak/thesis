
add_library(pseudo_boolean
        explanation_pseudo_boolean_constraint.cpp  watched_pseudo_boolean_constraint.cpp
        compute_propagations_up_to_literal_assignment.h  explanation_pseudo_boolean_constraint.h    watched_pseudo_boolean_constraint.h
        conflict_analyser_cutting_planes.cpp             propagator_pseudo_boolean_constraints.cpp  watcher_pseudo_boolean.h
        conflict_analyser_cutting_planes.h               propagator_pseudo_boolean_constraints.h    watch_list_pseudo_boolean.cpp
        database_pseudo_boolean.cpp                      reason_pseudo_boolean_constraint.cpp       watch_list_pseudo_boolean.h
        database_pseudo_boolean.h                        reason_pseudo_boolean_constraint.h
        )

target_link_libraries(pseudo_boolean propagator_generic)
