include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_CLI "Enable command line  programs" ON "ENABLE_LIBRARY" ON)
add_feature_info(ENABLE_CLI ENABLE_CLI "LimeSuite command line interface tools")
if (NOT ENABLE_CLI)
    return()
endif()

add_executable(limeDevice limeDevice.cpp)
set_target_properties(limeDevice PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
target_include_directories(limeDevice PUBLIC ${LIME_SUITE_INCLUDES})
target_link_libraries(limeDevice PUBLIC ${MAIN_LIBRARY_NAME})

add_executable(limeSPI limeSPI.cpp common.cpp)
set_target_properties(limeSPI PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
target_include_directories(limeSPI PUBLIC ${LIME_SUITE_INCLUDES})
target_link_libraries(limeSPI PUBLIC ${MAIN_LIBRARY_NAME})

add_executable(limeConfig limeConfig.cpp common.cpp)
set_target_properties(limeConfig PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
target_include_directories(limeConfig PUBLIC ${LIME_SUITE_INCLUDES})
target_link_libraries(limeConfig PUBLIC ${MAIN_LIBRARY_NAME})

add_executable(limeFLASH limeFLASH.cpp common.cpp)
set_target_properties(limeFLASH PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
target_include_directories(limeFLASH PUBLIC ${LIME_SUITE_INCLUDES})
target_link_libraries(limeFLASH PUBLIC ${MAIN_LIBRARY_NAME})

add_executable(limeTRX limeTRX.cpp common.cpp ${PROJECT_SOURCE_DIR}/external/kissFFT/kiss_fft.c)
set_target_properties(limeTRX PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
target_include_directories(limeTRX PUBLIC ${LIME_SUITE_INCLUDES})
target_link_libraries(limeTRX PUBLIC ${MAIN_LIBRARY_NAME})

install(TARGETS limeDevice limeSPI limeFLASH limeTRX DESTINATION bin)
