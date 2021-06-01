add_library(encoders i_encoder.cc
        i_encoder.h
        propagator_encoder.cc
        propagator_encoder.h
        generalized_totaliser.cc
        generalized_totaliser.h
        )
#target_link_libraries(encoders propagator_generic propagator_dynamic)
