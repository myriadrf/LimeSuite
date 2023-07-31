include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_EXAMPLES "Enable library example programs" ON "ENABLE_LIBRARY" ON)
add_feature_info(LimeSuiteExamples ENABLE_EXAMPLES "LimeSuite library API examples")
if (NOT ENABLE_EXAMPLES)
    return()
endif()

if (UNIX)
    find_package(Gnuplot)
    if (GNUPLOT_FOUND)
        add_definitions(-DUSE_GNU_PLOT)
        message(STATUS "GNUPlot Found")
    endif()
endif()

set(examplesOutputDir "${CMAKE_BINARY_DIR}/bin/examples")

add_executable(basicRX basicRX.cpp ${PROJECT_SOURCE_DIR}/external/kissFFT/kiss_fft.c)
set_target_properties(basicRX PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${examplesOutputDir})
target_link_libraries(basicRX ${MAIN_LIBRARY_NAME})

add_executable(dualRXTX dualRXTX.cpp)
set_target_properties(dualRXTX PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${examplesOutputDir})
target_link_libraries(dualRXTX ${MAIN_LIBRARY_NAME})

add_executable(basicTX basicTX.cpp)
set_target_properties(basicTX PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${examplesOutputDir})
target_link_libraries(basicTX PUBLIC ${MAIN_LIBRARY_NAME})
