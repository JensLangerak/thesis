add_library(encoders i_encoder.cc
        i_encoder.h
        propagator_encoder.cc
        propagator_encoder.h
        generalized_totaliser.cc
        generalized_totaliser.h
        generalized_totaliser_sum_helpers.cc
        generalized_totaliser_sum_helpers.h
        generalized_totaliser_sum_nodes.cc
        generalized_totaliser_sum_nodes.h
        generalized_totliser_sum_root.cc
        generalized_totliser_sum_root.h
        )
target_link_libraries(encoders propagator_generic propagator_dynamic pb_sum_propagator)
