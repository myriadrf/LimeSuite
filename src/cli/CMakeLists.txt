include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_CLI "Enable comman line programs" ON "ENABLE_LIBRARY" ON)
add_feature_info(LimeSuiteCLI ENABLE_CLI "LimeSuite command line tools")
if (NOT ENABLE_CLI)
    return()
endif()

add_executable(LimeDevice LimeDevice.cpp)
set_target_properties(LimeDevice PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
target_include_directories(LimeDevice PUBLIC ${LIME_SUITE_INCLUDES})
target_link_libraries(LimeDevice PUBLIC LimeSuite2 ${MAIN_LIBRARY_NAME})

add_executable(LimeSPI LimeSPI.cpp)
set_target_properties(LimeSPI PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
target_include_directories(LimeSPI PUBLIC ${LIME_SUITE_INCLUDES})
target_link_libraries(LimeSPI PUBLIC LimeSuite2 ${MAIN_LIBRARY_NAME})

add_executable(LimeConfig LimeConfig.cpp)
set_target_properties(LimeConfig PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
target_include_directories(LimeConfig PUBLIC ${LIME_SUITE_INCLUDES})
target_link_libraries(LimeConfig PUBLIC LimeSuite2 ${MAIN_LIBRARY_NAME})

add_executable(LimeFLASH LimeFLASH.cpp)
set_target_properties(LimeFLASH PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
target_include_directories(LimeFLASH PUBLIC ${LIME_SUITE_INCLUDES})
target_link_libraries(LimeFLASH PUBLIC LimeSuite2 ${MAIN_LIBRARY_NAME})

