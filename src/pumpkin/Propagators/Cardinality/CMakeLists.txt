add_library(cardinality propagator_cardinality.cc propagator_cardinality.h database_cardinality.cc database_cardinality.h
        watched_cardinality_constraint.cc watched_cardinality_constraint.h watch_list_cardinality.cc watch_list_cardinality.h
        watcher_cardinality_constraint.cc watcher_cardinality_constraint.h reason_cardinality_constraint.cc reason_cardinality_constraint.h
        explanation_cardinality_constraint.cc explanation_cardinality_constraint.h
        Encoders/totaliser_encoder.cc Encoders/totaliser_encoder.h Encoders/i_encoder.h Encoders/i_encoder.cc
        Encoders/sequential_encoder.cc Encoders/sequential_encoder.h Encoders/incremental_sequential_encoder.cc
        Encoders/incremental_sequential_encoder.h Encoders/propagator_encoder.cc Encoders/propagator_encoder.h
        Sum/database_sum.cc Sum/database_sum.h Sum/explanation_sum_constraint.cc Sum/explanation_sum_constraint.h
        Sum/propagator_sum.h Sum/propagator_sum.cc Sum/reason_sum_constraint.cc Sum/reason_sum_constraint.h
        Sum/watch_list_sum.cc Sum/watch_list_sum.h Sum/watched_sum_constraint.cc Sum/watched_sum_constraint.h
        Sum/watcher_sum_constraint.cc Sum/watcher_sum_constraint.h)
target_link_libraries(cardinality propagator_generic)