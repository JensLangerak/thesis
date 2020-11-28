add_library(cardinality_propagator
        database_cardinality.cc
        database_cardinality.h
        explanation_cardinality_constraint.cc
        explanation_cardinality_constraint.h
        propagator_cardinality.cc
        propagator_cardinality.h
        reason_cardinality_constraint.cc
        reason_cardinality_constraint.h
        watched_cardinality_constraint.cc
        watched_cardinality_constraint.h
        watcher_cardinality_constraint.cc
        watcher_cardinality_constraint.h
        watch_list_cardinality.cc
        watch_list_cardinality.h
        )
target_link_libraries(cardinality_propagator propagator_generic encoders propagator_dynamic)
