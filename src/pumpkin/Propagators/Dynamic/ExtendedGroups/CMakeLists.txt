add_library(pseudo_boolean_extended_groups
        database_extended_groups.cc
        database_extended_groups.h
        explanation_extended_groups.cc
        explanation_extended_groups.h
        extended_groups_adder.cc
        extended_groups_adder.h
        propagator_extended_groups.cc
        propagator_extended_groups.h
        watched_extended_groups_constraint.cc
        watched_extended_groups_constraint.h
        )
target_link_libraries(pseudo_boolean_extended_groups propagator_generic encoders propagator_dynamic)
