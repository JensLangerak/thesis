add_library(encoders i_encoder.cc
        i_encoder.h
        incremental_sequential_encoder.cc
        incremental_sequential_encoder.h
        propagator_encoder.cc
        propagator_encoder.h
        sequential_encoder.cc
        sequential_encoder.h
        totaliser_encoder.cc
        totaliser_encoder.h
        generalized_totaliser.cc generalized_totaliser.h static_generalized_totaliser.cc static_generalized_totaliser.h generalized_totaliser_sum_nodes.cc generalized_totaliser_sum_nodes.h generalized_totliser_sum_root.cc generalized_totliser_sum_root.h generalized_totaliser_sum_helpers.cc generalized_totaliser_sum_helpers.h)
target_link_libraries(encoders propagator_generic logger)