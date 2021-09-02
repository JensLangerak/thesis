add_library(sum_propagator
        database_sum.cc
        database_sum.h
        explanation_sum_constraint.cc
        explanation_sum_constraint.h
        propagator_sum.cc
        propagator_sum.h
        reason_sum_constraint.cc
        reason_sum_constraint.h
        watched_sum_constraint.cc
        watched_sum_constraint.h
        watcher_sum_constraint.cc
        watcher_sum_constraint.h
        watch_list_sum.cc
        watch_list_sum.h
        )
target_link_libraries(sum_propagator propagator_generic encoders)
