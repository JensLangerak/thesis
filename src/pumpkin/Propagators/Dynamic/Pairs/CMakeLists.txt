add_library(pseudo_boolean_pairs
        database_pb_pairs.h database_pb_pairs.cc
        explantion_pb_pairs_constraint.cc explantion_pb_pairs_constraint.h
        pb_pairs_adder.cc pb_pairs_adder.h
        propagator_pb_pairs.cc propagator_pb_pairs.h
        watched_pb_pairs_constraint.cc watched_pb_pairs_constraint.h)
target_link_libraries(pseudo_boolean_pairs propagator_generic encoders propagator_dynamic)
