include(FeatureSummary)
include(CMakeDependentOption)

cmake_dependent_option(ENABLE_UTILITIES "Enable utility programs" ON "ENABLE_LIBRARY" OFF)

if (NOT ENABLE_UTILITIES)
    return()
endif()

# add_executable(pll_sweep pll_sweep.cpp)
# set_target_properties(pll_sweep PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
# target_link_libraries(pll_sweep ${MAIN_LIBRARY_NAME})

add_executable(rfTest rfTest.cpp)
set_target_properties(rfTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
target_include_directories(rfTest PUBLIC ${LIME_SUITE_INCLUDES})
target_link_libraries(rfTest PUBLIC ${MAIN_LIBRARY_NAME})





