file(GLOB _SRCS "*.h" "*.cpp")

add_subdirectory(Utilities)
add_subdirectory(Engine)
add_subdirectory(Propagators)
add_subdirectory("Pseudo-Boolean Encoders")
add_library(pumpkin INTERFACE)
target_link_libraries(pumpkin INTERFACE engine utilities propagators pseudo_boolean_encoders)
add_executable(pumpkin_main main.cpp)
target_link_libraries(pumpkin_main pumpkin)